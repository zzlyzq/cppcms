///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_HTTP_CONTEXT_H
#define CPPCMS_HTTP_CONTEXT_H

#include <cppcms/defs.h>
#include <booster/hold_ptr.h>
#include <booster/intrusive_ptr.h>
#include <booster/shared_ptr.h>
#include <booster/enable_shared_from_this.h>
#include <booster/callback.h>
#include <booster/noncopyable.h>
#include <locale>

namespace cppcms {

	class service;
	class application;
	class cache_interface;
	class session_interface;
	namespace json { class value; }
	namespace impl { namespace cgi { class connection; } }

	///
	/// \brief This namespace represent classes that are directly connected to handing HTTP requests and responses
	///
	namespace http {
		class request;
		class response;

		///
		/// \brief context is a central class that holds all specific connection related information.
		/// It encapsulates CGI request and response, cache, session and locale information
		///
		/// Instance of this class is created upon client requests, it provides access to all
		/// connection related interfaces. This class is unique per each applications hierarchy 
		/// and destroyed when HTTP request/response is completed
		///
		
		class CPPCMS_API context : 
			public booster::noncopyable,
			public booster::enable_shared_from_this<context>
		{
		public:
			/// \cond INTERNAL

			context(booster::shared_ptr<impl::cgi::connection> conn);
			~context();
			impl::cgi::connection &connection();
			void run();

			/// \endcond

			///
			/// Get an interface to HTTP request
			///
			http::request &request();

			///
			/// Get an interface to HTTP response
			///
			http::response &response();

			///
			/// Get global settings. Same as cppcms::service::settings
			///
			json::value const &settings();

			///
			/// Get an interface to CppCMS Cache
			///
			cache_interface &cache();

			///
			/// Get an interface to current session
			///
			/// Note, when using asynchronous CppCMS applications, session data is not fetched
			/// and is not updated, because session access may be not cheap, So when using
			/// session_interface in asynchronous application make sure you call session_inerface::load
			/// member function
			///
			session_interface &session();

			///
			/// Get current context locale
			///
			std::locale locale();

			///
			/// Set locale explicitly. Note, it changes the locale of the response().out() stream as
			/// well
			///
			void locale(std::locale const &new_locale);

			///
			/// Set locale by name. Similar to locale(service().generator(name)).
			///
			/// Note: it changes the locale of the response().out() stream as well
			///
			void locale(std::string const &name);
			
			///
			/// Get the central service instance
			///
			cppcms::service &service();

			///
			/// Get current views skin name
			///
			std::string skin();

			///
			/// Set current views skin name
			///
			void skin(std::string const &name);


			typedef enum {
				operation_completed, ///< Asynchronous operation completed successfully 
				operation_aborted    ///< Asynchronous operation was canceled
			} completion_type; 

			typedef booster::callback<void(completion_type)> handler;

			///
			/// Send all pending output data to the client and
			/// finalize the connection. Note, you can't use this 
			/// object for communication any more.
			///
			void complete_response();

			///
			/// Send all pending output data to the client and
			/// finalize the connection. Note, you can't use this 
			/// object for communication any more.
			///
			void async_complete_response();
			
			///
			/// Send all pending data to user, when operation is complete
			/// call handler \a h with status.
			///
			/// Note: if the status is operation_aborted, you can't use
			/// this connection any more, the peer gone.
			///

			void async_flush_output(handler const &h);

			///
			/// Set handler for peer reset events. It is useful to cleanup
			/// connections that had timeout or just disconnected by user
			///
			/// Notes: 
			///
			/// -# if async_complete_response was called, handler would not
			///    be called any more.
			/// -# If async_flush_output fails, this does not mean that 
			///    this handler would be called as well, so you need to check both
			///
			void async_on_peer_reset(booster::callback<void()> const &h);
		private:
			void on_request_ready(bool error);
			static void dispatch(booster::intrusive_ptr<application> app,std::string url,bool syncronous);
			void try_restart(bool e);
			booster::shared_ptr<context> self();

			struct _data;
			booster::hold_ptr<_data> d;
			booster::shared_ptr<impl::cgi::connection> conn_;
		};

	}

};

#endif
