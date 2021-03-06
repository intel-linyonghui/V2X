#include "DefaultChannel.h"
#include "Util.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

using namespace std;

DefaultChannel::DefaultChannel()
{
}

//误码模拟
void DefaultChannel::bitSimulation(SliceMessage &sMsg)
{
	LARGE_INTEGER seed;

	QueryPerformanceFrequency(&seed);
	QueryPerformanceCounter(&seed);
	srand(seed.QuadPart);//初始化一个以微秒为单位的时间种子

	//srand((unsigned)time(NULL));
	int a = rand() % BIT_RATE;
	cout << "bitsimulation a: " << a << endl;
	if (a == 0)
	{
		sMsg.setError(true);
	}
	else
	{
		sMsg.setError(false);
	}
	cout << "error: " << sMsg.getError() << endl;
}

//时延模拟
void DefaultChannel::delaySimulation(SliceMessage &sMsg, int index, list<CommunicationNode> communicationNodes, int simulation_count)
{
	CommunicationNode node = getNode(communicationNodes, sMsg.getSourceAddress());
	float t = (sMsg.getSize() * 8) / node.getTRate();
	t = t + (float)((0.1 / simulation_count) * (index - 1));
	sMsg.setDelay(t);
	cout << "delay: " << sMsg.getDelay() << endl;
}