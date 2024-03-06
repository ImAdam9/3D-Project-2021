#ifndef TIMERWRAP_H
#define TIMERWRAP_H
#include <profileapi.h> // timer
#include <stdint.h>     //uint64_t etc
#include <DirectXMath.h>

class TimerWrap
{
private:
	uint64_t frequency;
	uint64_t offset;
	float timeInterval;
	double GetSeconds();
public:
	TimerWrap();
	~TimerWrap();

	void InitTimer();
	void UpdateTimeInterval();
	float GetTimeInterval();
};
#endif