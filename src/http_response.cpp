///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include "cgi_api.h"
#include <cppcms/http_response.h>
#include <cppcms/http_context.h>
#include <cppcms/http_request.h>
#include <cppcms/http_cookie.h>
#include "http_protocol.h"
#include "cached_settings.h"
#include <cppcms/json.h>
#include <cppcms/cppcms_error.h>
#include <cppcms/service.h>
#include <cppcms/config.h>
#include <cppcms/localization.h>
#include <cppcms/util.h>
#include <cppcms/session_interface.h>

#include <booster/log.h>
#include <booster/posix_time.h>
#include <iostream>
#include <sstream>
#include <iterator>
#include <map>

#include <cppcms_boost/iostreams/stream.hpp>
#include <cppcms_boost/iostreams/filtering_stream.hpp>
#ifndef CPPCMS_NO_GZIP
#include <cppcms_boost/iostreams/filter/gzip.hpp>
#endif
#include <cppcms_boost/iostreams/tee.hpp>
namespace boost = cppcms_boost;

namespace cppcms { namespace http {

namespace {

	bool string_i_comp(std::string const &left,std::string const &right)
	{
		return http::protocol::compare(left,right) < 0;
	}
	template<typename T>
	std::string itoa(T v)
	{
		std::ostringstream ss;
		ss.imbue(std::locale::classic());
		ss<<v;
		return ss.str();
	}
} // anon



namespace  {
	class output_device : public boost::iostreams::sink {
		booster::weak_ptr<impl::cgi::connection> conn_;
	public:
		output_device(impl::cgi::connection *conn) : 
			conn_(conn->shared_from_this())
		{
		}
		std::streamsize write(char const *data,std::streamsize n)
		{
			if(n==0)
				return 0;
			booster::shared_ptr<impl::cgi::connection> c = conn_.lock();
			if(!c) {
				throw std::runtime_error("no writer");
			}
			
			booster::system::error_code e;

			size_t res = c->write(data,n,e);
			if(e) {
				BOOSTER_WARNING("cppcms") << "Failed to write response:" << e.message();
				conn_.reset();
			}
			return res;
		}
	};
}

struct response::_data {
	typedef bool (*compare_type)(std::string const &left,std::string const &right);
	typedef std::map<std::string,std::string,compare_type> headers_type;
	headers_type headers;
	std::vector<cookie> cookies;
	std::ostringstream cached;
	std::ostringstream buffered;
	boost::iostreams::stream<output_device> output;
	boost::iostreams::filtering_ostream filter;

	_data(impl::cgi::connection *conn) : 
		headers(string_i_comp),
		output(output_device(conn),conn->service().cached_settings().service.output_buffer_size)
	{
	}
};


response::response(context &context) :
	d(new _data(&context.connection())),
	context_(context),
	stream_(0),
	io_mode_(normal),
	disable_compression_(0),
	ostream_requested_(0),
	copy_to_cache_(0),
	finalized_(0)
{
	set_content_header("text/html");
	if(context_.service().cached_settings().service.disable_xpowered_by==false) {
		set_header("X-Powered-By", CPPCMS_PACKAGE_NAME "/" CPPCMS_PACKAGE_VERSION);
	}
}

response::~response()
{
}

void response::set_content_header(std::string const &content_type)
{
	if(context_.service().cached_settings().localization.disable_charset_in_content_type) {
		set_header("Content-Type",content_type);
	}
	else {
		std::string charset=std::use_facet<locale::info>(context_.locale()).encoding();
		set_header("Content-Type",content_type+"; charset="+charset);
	}
}
void response::set_html_header()
{
	set_content_header("text/html");
}
void response::set_xhtml_header()
{
	set_content_header("text/xhtml");
}
void response::set_plain_text_header()
{
	set_content_header("text/plain");
}
void response::set_redirect_header(std::string const &loc,int s)
{
	location(loc);
	status(s);
}
void response::set_cookie(cookie const &cookie)
{
	d->cookies.push_back(cookie);
}

void response::set_header(std::string const &name,std::string const &value)
{
	if(value.empty())
		d->headers.erase(name);
	else
		d->headers[name]=value;
}

void response::finalize()
{
	if(!finalized_) {
		out()<<std::flush;
		try {
			d->filter.reset();
		}
		catch(...) {
			// device may throw!
		}
		finalized_=1;
	}
}

std::string response::get_header(std::string const &name)
{
	_data::headers_type::const_iterator p=d->headers.find(name);
	if(p!=d->headers.end())
		return p->second;
	return std::string();
}

void response::erase_header(std::string const &name)
{
	d->headers.erase(name);
}

bool response::need_gzip()
{
#ifndef CPPCMS_NO_GZIP
	if(disable_compression_)
		return false;
	if(io_mode_!=normal)
		return false;
	if(context_.service().cached_settings().gzip.enable==false)
		return false;
	if(strstr(context_.request().cgetenv("HTTP_ACCEPT_ENCODING"),"gzip")==0)
		return false;
	if(!get_header("Content-Encoding").empty())
		// User had defined its own content encoding
		// he may compress data on its own... disable compression
		return false;
	std::string const content_type=get_header("Content-Type");
	if(protocol::is_prefix_of("text/",content_type))
		return true;
	return false;
#else
	return false;
#endif
}

response::io_mode_type response::io_mode()
{
	return io_mode_;
}

void response::io_mode(response::io_mode_type mode)
{
	if(ostream_requested_)
		throw cppcms_error("Can't set mode after requesting output stream");
	io_mode_=mode;
}

void response::write_http_headers(std::ostream &out)
{
	context_.session().save();
	
	_data::headers_type::const_iterator p = d->headers.end();

	if(context_.service().cached_settings().service.generate_http_headers) {
		p=d->headers.find("Status");
		if(p == d->headers.end())
			out << "HTTP/1.0 200 Ok\r\n";
		else
			out << "HTTP/1.0 " << p->second <<"\r\n";
	}
	
	for(_data::headers_type::const_iterator h=d->headers.begin();h!=d->headers.end();++h) {
		if(h==p)
			continue;
		out<<h->first<<": "<<h->second<<"\r\n";
	}
	
	for(unsigned i=0;i<d->cookies.size();i++) {
		out<<d->cookies[i]<<"\r\n";
	}

	out<<"\r\n";
	out<<std::flush;
}


void response::copy_to_cache()
{
	copy_to_cache_=1;
}

std::string response::copied_data()
{
	if(!copy_to_cache_ || !ostream_requested_)
		return std::string();
	return d->cached.str();	
}

std::ostream &response::out()
{
	using namespace boost::iostreams;

	if(ostream_requested_)
		return *stream_;
	if(finalized_)
		throw cppcms_error("Request for output stream for finalized request is illegal");
	
	if(io_mode_ == asynchronous || io_mode_ == asynchronous_raw) 
		stream_ = &d->buffered;
	else 
		stream_ = &d->output;
	
	ostream_requested_=1;
	
	bool use_filter = false;

	#ifndef CPPCMS_NO_GZIP
	bool gzip = need_gzip();
	
	if(gzip) {
		content_encoding("gzip");
	}
	#endif

	// Now we shoulde write headers -- before comrpession
	if(io_mode_ != raw && io_mode_ != asynchronous_raw)
		write_http_headers(*stream_);
	
	#ifndef CPPCMS_NO_GZIP
	if(gzip) {
		gzip_params params;

		int level=context_.service().cached_settings().gzip.level;
		if(level!=-1)
			params.level=level;
		int buffer=context_.service().cached_settings().gzip.buffer;
		if(buffer!=-1)
			d->filter.push(gzip_compressor(params,buffer));
		else
			d->filter.push(gzip_compressor(params));

		use_filter = true;
	}
	#endif
	
	if(copy_to_cache_) {
		d->filter.push(tee_filter<std::ostream>(d->cached));
		use_filter = true;
	}
	
	if(use_filter) {
		d->filter.push(*stream_);
		stream_ = &d->filter;
	}

	stream_->imbue(context_.locale());
	return *stream_;
}

std::string response::get_async_chunk()
{
	std::string result=d->buffered.str();
	d->buffered.str("");
	return result;
}

bool response::some_output_was_written()
{
	return ostream_requested_;
}

char const *response::status_to_string(int status)
{
	switch(status) {
	case 100: return "Continue";
 	case 101: return "Switching Protocols";
 	case 200: return "OK";
 	case 201: return "Created";
 	case 202: return "Accepted";
 	case 203: return "Non-Authoritative Information";
 	case 204: return "No Content";
 	case 205: return "Reset Content";
 	case 206: return "Partial Content";
 	case 300: return "Multiple Choices";
 	case 301: return "Moved Permanently";
 	case 302: return "Found";
 	case 303: return "See Other";
 	case 304: return "Not Modified";
 	case 305: return "Use Proxy";
 	case 307: return "Temporary Redirect";
 	case 400: return "Bad Request";
 	case 401: return "Unauthorized";
 	case 402: return "Payment Required";
 	case 403: return "Forbidden";
 	case 404: return "Not Found";
 	case 405: return "Method Not Allowed";
 	case 406: return "Not Acceptable";
 	case 407: return "Proxy Authentication Required";
 	case 408: return "Request Time-out";
 	case 409: return "Conflict";
 	case 410: return "Gone";
 	case 411: return "Length Required";
 	case 412: return "Precondition Failed";
 	case 413: return "Request Entity Too Large";
 	case 414: return "Request-URI Too Large";
 	case 415: return "Unsupported Media Type";
 	case 416: return "Requested range not satisfiable";
 	case 417: return "Expectation Failed";
 	case 500: return "Internal Server Error";
 	case 501: return "Not Implemented";
 	case 502: return "Bad Gateway";
 	case 503: return "Service Unavailable";
 	case 504: return "Gateway Time-out";
 	case 505: return "HTTP Version not supported";
	default:  return "Unknown";
	}
}

void response::make_error_response(int stat,std::string const &msg)
{
	status(stat);
	out() <<"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"\n"
		"	\"http://www.w3.org/TR/html4/loose.dtd\">\n"
		"<html>\n"
		"  <head>\n"
		"    <title>"<<stat<<" &mdash; "<< http::response::status_to_string(stat)<<"</title>\n"
		"  </head>\n"
		"  <body>\n"
		"    <h1>"<<stat<<" &mdash; "<< http::response::status_to_string(stat)<<"</h1>\n";
	if(!msg.empty()) {
		out()<<"    <p>"<<util::escape(msg)<<"</p>\n";
	}
	out()<<	"  </body>\n"
		"</html>\n"<<std::flush;
}


void response::accept_ranges(std::string const &s) { set_header("Accept-Ranges",s); }
void response::age(unsigned seconds) { set_header("Age",itoa(seconds)); }
void response::allow(std::string const &s) { set_header("Allow",s); }
void response::cache_control(std::string const &s) { set_header("Cache-Control",s); }
void response::content_encoding(std::string const &s) { set_header("Content-Encoding",s); } 
void response::content_language(std::string const &s) { set_header("Content-Language",s); }
void response::content_length(unsigned long long len)
{
	set_header("Content-Length",itoa(len));
}
void response::content_location(std::string const &s) { set_header("Content-Location",s); }
void response::content_md5(std::string const &s) { set_header("Content-MD5",s); }
void response::content_range(std::string const &s) { set_header("Content-Range",s); }
void response::content_type(std::string const &s) { set_header("Content-Type",s); }
void response::date(time_t t) { set_header("Date",make_http_time(t)); }
void response::etag(std::string const &s) { set_header("ETag",s); }
void response::expires(time_t t) { set_header("Expires",make_http_time(t)); }
void response::last_modified(time_t t) { set_header("Last-Modified",make_http_time(t)); }
void response::location(std::string const &s) { set_header("Location",s); }
void response::pragma(std::string const &s) { set_header("Pragma",s); }
void response::proxy_authenticate(std::string const &s) { set_header("Proxy-Authenticate",s); }
void response::retry_after(unsigned n) { set_header("Retry-After",itoa(n)); }
void response::retry_after(std::string const &s) { set_header("Retry-After",s); }
void response::status(int code)
{
	status(code,status_to_string(code));
}
void response::status(int code,std::string const &message)
{
	set_header("Status",itoa(code)+" "+message);
}
void response::trailer(std::string const &s) { set_header("Trailer",s); }
void response::transfer_encoding(std::string const &s) { set_header("Transfer-Encoding",s); }
void response::vary(std::string const &s) { set_header("Vary",s); }
void response::via(std::string const &s) { set_header("Via",s); }
void response::warning(std::string const &s) { set_header("Warning",s); }
void response::www_authenticate(std::string const &s) { set_header("WWW-Authenticate",s); }

std::string response::make_http_time(time_t t)
{
	// RFC 2616
	// "Sun, 06 Nov 1994 08:49:37 GMT"

	std::tm tv=booster::ptime::universal_time(booster::ptime(t));

	std::ostringstream ss;
	std::locale C=std::locale::classic();
	ss.imbue(C);

	std::time_put<char> const &put = std::use_facet<std::time_put<char> >(C);
	char const format[]="%a, %d %b %Y %H:%M:%S GMT"; 
	put.put(ss,ss,' ',&tv,format,format+sizeof(format)-1);
	return ss.str();
}


} } // http::cppcms
