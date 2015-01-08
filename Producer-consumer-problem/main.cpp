#include <iostream>

#include "test.hpp"

using namespace concurrency;

int main(int argc, char* argv[])
{
	// create producers
	std::vector<kinect> kinects(3);
	// create processor's outputs
	std::vector<kinectdatabuffer*> kdbs = { &kinects[0].output(), &kinects[1].output(), &kinects[2].output() };

	// create processor
	tracker t(kdbs);
	// create consumer
	renderer r(t.output());

	// agent list
	agent* agents[] = { &kinects[0], &kinects[1], &kinects[2], &t, &r };

	// start all agents
	std::for_each(std::begin(agents), std::end(agents), [&](agent* agent) { agent->start(); });

	agent::wait_for_all(sizeof(agents) / sizeof(agent*), agents);
	agent::wait(&kinects[0]);
	agent::wait(&kinects[1]);
	agent::wait(&kinects[2]);
	agent::wait(&t);
	agent::wait(&r);

	return 0;
}