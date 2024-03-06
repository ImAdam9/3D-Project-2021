#include "../Header/TimerWrap.h"

double TimerWrap::GetSeconds()
{
	uint64_t counter = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&counter);
	return (double)(counter - offset) / frequency;
}

TimerWrap::TimerWrap()
{
	this->frequency = 1000000;
	this->offset = 0;
	this->timeInterval = 0.0f;
}

TimerWrap::~TimerWrap()
{
}

void TimerWrap::InitTimer()
{
	this->frequency = 1000; // QueryPerformanceCounter default 
	QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
	QueryPerformanceCounter((LARGE_INTEGER*)&offset);
}

void TimerWrap::UpdateTimeInterval()
{
	this->timeInterval = this->GetSeconds() * DirectX::XM_PIDIV4;
}

float TimerWrap::GetTimeInterval()
{
	return this->timeInterval;
}


