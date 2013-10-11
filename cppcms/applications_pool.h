///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_APPLICATIONS_POOL_H
#define CPPCMS_APPLICATIONS_POOL_H

#include <cppcms/defs.h>
#include <booster/noncopyable.h>
#include <booster/hold_ptr.h>
#include <booster/intrusive_ptr.h>

#include <memory>
#include <string>

namespace cppcms {

	class application;
	class service;
	class mount_point;
	
	///
	/// \brief Application pool is the central class that holds user created applications
	///
	/// Form the user perspective this class provides an API for mounting user application to the CppCMS service.
	///
	/// There are two kind of \a mount member functions, that allow:
	/// 
	/// - Mounting a \a factory of user applications -- for execution of synchronous requests by multiple
	///   instances of application.
	/// - Mounting single application -- for processing asynchronous requests by single instance of an application
	///
	/// The life cycle of synchronous application is defined by application pool itself, and the life cycle
	/// of asynchronous depends on its own reference count.
	///
	/// This class is thread safe and can be accessed from multiple threads simultaneously.
	///
	class CPPCMS_API applications_pool {
	public:

		///
		/// \brief a base class for user application factories
		///
		struct factory : public booster::noncopyable {
			///
			/// Returns newly created instance of an application.
			///
			virtual std::auto_ptr<application> operator()(service &) const = 0;
			virtual ~factory(){}
		};

		///
		/// Mount an application factory \a aps for processing of any incoming requests. Application
		/// would receive PATH_INFO CGI variable for URL matching.
		///
		/// This member function is thread safe.
		///
		void mount(std::auto_ptr<factory> aps);
		
		///
		/// Mount an application factory \a app  by mount_point \a point application matching and
		/// URL selection rules
		///
		/// This member function is thread safe.
		///

		void mount(std::auto_ptr<factory> aps,mount_point const &point);

		///
		/// Mount an asynchronous application \a app for processing of any incoming requests. Application
		/// would receive PATH_INFO CGI variable for URL matching.
		///
		/// This member function is thread safe.
		///
		void mount(booster::intrusive_ptr<application> app);
		///
		/// Mount an asynchronous application \a app  by mount_point \a point application matching and
		/// URL selection rules
		///
		/// This member function is thread safe.
		///
		void mount(booster::intrusive_ptr<application> app,mount_point const &point);


		/// \cond INTERNAL

		booster::intrusive_ptr<application> 
		get(char const *h,char const *s,char const *path_info,std::string &match);

		void put(application *app);
		applications_pool(service &srv,int pool_size_limit);
		~applications_pool();

		/// \endcond

	private:
		struct basic_app_data;
		struct app_data;
		struct long_running_app_data;
		struct _data;
		service *srv_;
		booster::hold_ptr<_data> d;
	};

	/// \cond INTERNAL 
	namespace details {
		template<typename T>
		struct simple_factory0 : public applications_pool::factory
		{
			std::auto_ptr<application> operator()(service &s) const
			{
				std::auto_ptr<application> app(new T(s));
				return app;
			}
		};
		template<typename T,typename P1>
		struct simple_factory1 : public applications_pool::factory
		{
			simple_factory1(P1 p1) : p1_(p1) {}
			P1 p1_;
			std::auto_ptr<application> operator()(service &s) const
			{
				std::auto_ptr<application> app(new T(s,p1_));
				return app;
			}
		};
		template<typename T,typename P1,typename P2>
		struct simple_factory2 : public applications_pool::factory 
		{
			simple_factory2(P1 p1,P2 p2) : p1_(p1),p2_(p2) {}
			P1 p1_;
			P2 p2_;
			std::auto_ptr<application> operator()(service &s) const
			{
				std::auto_ptr<application> app(new T(s,p1_,p2_));
				return app;
			}
		};
	} // details

	/// \endcond

	///
	/// Create application factory for application of type T, such as T has a constructor
	/// T::T(cppcms::service &s);
	///
	template<typename T>
	std::auto_ptr<applications_pool::factory> applications_factory()
	{
		std::auto_ptr<applications_pool::factory> f(new details::simple_factory0<T>);
		return f;
	}
	
	///
	/// Create application factory for application of type T, such as T has a constructor
	/// T::T(cppcms::service &s,P1);
	///
	template<typename T,typename P1>
	std::auto_ptr<applications_pool::factory> applications_factory(P1 p1)
	{
		std::auto_ptr<applications_pool::factory> f(new details::simple_factory1<T,P1>(p1));
		return f;
	}
	
	///
	/// Create application factory for application of type T, such as T has a constructor
	/// T::T(cppcms::service &s,P1,P2);
	///
	template<typename T,typename P1,typename P2>
	std::auto_ptr<applications_pool::factory> applications_factory(P1 p1,P2 p2)
	{
		std::auto_ptr<applications_pool::factory> f(new details::simple_factory2<T,P1,P2>(p1,p2));
		return f;
	}


} // cppcms



#endif
