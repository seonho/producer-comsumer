/*
 *	@file		test.hpp
 *	@brief		Implement dummy struct and test class for producer-consumer problem test.
 *	@author		seonho.oh@gmail.com
 *	@date		2015-01-07
 */

#pragma once

#include "basic_agents.hpp"
#include <Windows.h>

//! dummy data type for kinect
struct kinectdata
{
};

//! dummy data type for tracker
struct data
{
};

typedef concurrency::unbounded_buffer<kinectdata *> kinectdatabuffer;

template <typename C>
typename std::enable_if<std::is_pointer<typename C::value_type>::value, void>::type safe_release(C& c)
{
	std::for_each(c.begin(), c.end(), [&](typename C::value_type ptr) { delete ptr; });
}

class kinect : public basic_producer < kinectdata * >
{
public:
	kinect() : count_(0)
	{
		max_count_ = rand() % 10 + 5; // 임의의 갯수 만큼 만들어봅니다.
	}
protected:
	// create data and push to buffer
	virtual kinectdata* create()
	{
		std::cout << "create kinect." << std::endl;
		count_++;
		return new kinectdata;
	}

	virtual bool terminate()
	{
		// terminate condition
		return count_ >= max_count_;
	}
private:
	size_t max_count_;
	size_t count_;
};

//! combine multiple kinect data
class tracker : public basic_processor_many_to_one < kinectdata*, data* >
{
public:
	typedef basic_processor_many_to_one<kinectdata*, data*>	base_class;

	tracker(std::vector<typename base_class::input_container_type*> inputs) : base_class(inputs) {}
protected:
	base_class::output_type process(std::vector<typename base_class::input_type>& values)
	{
		safe_release(values);
		std::cout << "process data and create a new data." << std::endl;
		return new data;
	}
};

class renderer : public basic_consumer < data* >
{
public:
	typedef basic_consumer <data*> base_class;
	renderer(base_class::input_container_type& container) : base_class(container) {}
protected:
	virtual void use(data*& value)
	{
		delete value;
		std::cout << "consume a data." << std::endl;
	}
};