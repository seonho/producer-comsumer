/*
 *	@file		basic_agents.hpp
 *	@brief		Implement abstract producer, consumer and processor classes
 *	@author		seonho.oh@gmail.com
 *	@date		2015-01-07
 */

#pragma once

#include <ppl.h>
#include <agents.h>

namespace concurrency_ext
{
	// drain concurrent container
	template <typename container_type>
	typename std::enable_if<std::is_pointer<typename container_type::source_type>::value, void>::type drain(container_type& buf)
	{
		while (true) {
			container_type::source_type ptr = concurrency::receive(buf);

			if (ptr == nullptr)
				break;
			delete ptr;
		}
	}
};

//! abstract class for basic producer.
template <typename OutputType, typename ContainerType = concurrency::unbounded_buffer<OutputType> >
class basic_producer : public concurrency::agent
{
public:
	typedef OutputType		output_type;
	typedef ContainerType	output_container_type;
	
	output_container_type& output() { return output_; }

protected:
	void run()
	{
		while (true) {
			if (terminate())
				break;

			// create product
			concurrency::send(output_, create());
		}

		// send nullptr for finish message
		concurrency::send(output_, static_cast<output_type>(nullptr));

		done();
	}

	virtual output_type create() = 0;
	virtual bool terminate() = 0;

private:
	// holds outgoing messages.
	output_container_type	output_;
};

//! abstract constructor for basic consumer.
template <typename InputType, typename ContainerType = concurrency::unbounded_buffer<InputType> >
class basic_consumer : public concurrency::agent
{
	//static_assert(std::is_pointer<InputType>::value, "InputType mist be a pointer type.");
public:
	typedef InputType		input_type;
	typedef ContainerType	input_container_type;

	basic_consumer(input_container_type& input) : input_(input)	{}

protected:
	void run()
	{
		while (true) {
			// receive message from the input message buffer.
			input_type value = concurrency::receive(input_);

			if (value == nullptr) break;

			use(value);
		}

		done();
	}

	//! consume an input message.
	virtual void use(input_type& input) = 0;
private:
	// holds incoming messages.
	input_container_type&	input_;
};

//! basic one vs. one processor.
template <typename InputType, typename OutputType,
	typename InputContainerType = concurrency::unbounded_buffer<InputType>,
	typename OutputContainerType = concurrency::unbounded_buffer<OutputType> >
class basic_processor : public concurrency::agent
{
public:
	typedef InputType			input_type;
	typedef OutputType			output_type;
	typedef InputContainerType	input_container_type;
	typedef OutputContainerType	output_container_type;

	basic_processor(input_container_type& input) : input_(input) {}

	output_container_type& output() { return output_; }

protected:
	void run()
	{
		while (true) {
			// read from the input message buffer.
			input_type value = concurrency::receive(input_);

			if (value == nulltpr)
				break;

			output_type result = process(value);

			// write the result to the output message buffer
			concurrency::send(output_, result);
		}

		// send nullptr
		concurrency::send(output_, static_cast<output_type>(nullptr));

		done();
	}

	virtual output_type process(input_type& input) = 0;

private:
	// holds incoming messages.
	input_container_type&	input_;
	// holds outgoing messages.
	output_container_type	output_;
};

//! basic one vs. one processor.
template <typename InputType, typename OutputType,
	typename InputContainerType = concurrency::unbounded_buffer<InputType>,
	typename OutputContainerType = concurrency::unbounded_buffer<OutputType> >
class basic_processor_many_to_one : public concurrency::agent
{
public:
	typedef InputType			input_type;
	typedef OutputType			output_type;
	typedef InputContainerType	input_container_type;
	typedef OutputContainerType	output_container_type;

	basic_processor_many_to_one(std::vector<input_container_type*> inputs) : inputs_(inputs) {}

	output_container_type& output() { return output_; }

protected:
	void run()
	{
		while (true) {
			// read from the input message buffer.
			std::vector<input_type> values(inputs_.size());

			std::transform(inputs_.begin(), inputs_.end(), values.begin(), [&](input_container_type* input) {
				return concurrency::receive(*input);
			});
			
			if (std::any_of(values.begin(), values.end(), [](input_type value) { return value == nullptr; })) {

				std::cout << "destory all input buffers." << std::endl;

				// drain all buffers
				for (size_t i = 0; i < values.size(); i++) {
					if (values[i] != nullptr) {
						delete values[i];
						concurrency_ext::drain(*inputs_[i]);
					}
				}

				break;
			}

			output_type result = process(values);

			// write the result to the output message buffer
			concurrency::send(output_, result);
		}

		// send nullptr
		concurrency::send(output_, static_cast<output_type>(nullptr));

		done();
	}

	virtual output_type process(std::vector<input_type>& values) = 0;

private:
	// holds incoming messages.
	std::vector<input_container_type*>	inputs_;
	// holds outgoing messages.
	output_container_type				output_;
};