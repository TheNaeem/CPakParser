#pragma once

class FRunnable
{
public:
	virtual bool Init()
	{
		return true;
	}

	virtual uint32_t Run() = 0;
	virtual void Stop() { }
	virtual void Exit() { }

	virtual ~FRunnable() { }
};