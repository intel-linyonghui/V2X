#include "MessageTransferManagement.h"
#include "Util.h"
#include "DelieveQueue.h"
#include "MessageCheck.h"
#include "Protocol.h"
#include "constUtil.h"
#include "DefaultChannel.h"
#include <queue>

using namespace std;

//####################################################################################
//消息处理模块主方法，调用该方法可以模拟消息的处理过程
//index表示的是第几次的模拟处理过程，用于计算消息时延
//####################################################################################
void MessageTransferManagement::messageTransferManagement(int index, Protocol &protocol, list<CommunicationNode> &communicationNodes, DelieveQueue &dQueue, Channel &channel, int simulation_count)
{
	//1. 处理当前发送队列，如果存在退避，将退避时间-1
	protocol.preBackOff(communicationNodes);
	//2. 获取所有非空且当前不退避的发送队列
	list<CommunicationNode> *sendNode = new list<CommunicationNode>;
	sendNode = getNonemptySendQueueList(communicationNodes);

	//3. 选择发送队列的队首消息进入待送达队列，其余消息退避
	seleceMessage(*sendNode, dQueue);

	cout << "dQueue's size:" << dQueue.getQueueSize() << endl;

	//4. 退避
	backOffAll(*sendNode, protocol);
	//5. 将所有待送达队列中的消息经物理信道模拟
	while (!dQueue.isEmpty())
	{
		//从待送达队列中取出一条消息
		SliceMessage sMessage = dQueue.getMessage();
		//获取该消息的所有接收对象
		list<CommunicationNode> recvNodes = getAllReceiveNodes(sMessage, communicationNodes);

		if (recvNodes.size() == 0) {
			cout << "can't reach any recv nodes" << endl;
		}

		//遍历所有接收对象
		list<CommunicationNode>::iterator iter;
		for (iter = recvNodes.begin(); iter != recvNodes.end(); ++iter)
		{
			if (reachCheck(sMessage, (*iter), communicationNodes))
			{//消息可以到达当前的接收端
				//对消息进行物理信道模拟
				cout << "can reach node " << (*iter).getNodeID() << endl;
				channelSimulation(sMessage, index, communicationNodes, channel, simulation_count);
				if (!errorCheck(sMessage))
				{//消息正确,送入接收队列
					sendToReceiveQueue(sMessage, communicationNodes);
				}
				else {
					//消息误码，丢弃该消息
					cout << "There are errors in the message, so abundant it！" << endl;
					break;
				}
			}
			else {
				cout << "can't reach node " << (*iter).getNodeID() << endl;
			}
		}
	}
	delete sendNode;
}

//#######################################################################################
//从非空的发送队列中随机选取7个消息送入待送达队列，其余消息退避
//#######################################################################################
void MessageTransferManagement::seleceMessage(list<CommunicationNode> &communicationNode, DelieveQueue &dQueue)
{
	list<CommunicationNode>::iterator iter;
	if (communicationNode.size() <= CHANNEL_COUNT)
	{//非空发送队列数小于等于7个，直接取其队头的消息
		for (iter = communicationNode.begin(); iter != communicationNode.end(); ++iter)
		{
			//将该发送队列设置为不退避
			(*(*iter).getSendQueue()).setBackOff(false);
			//获取其队首的消息送入待送达队列
			sendToDelieveQueue(*(*(*iter).getSendQueue()).getMessage(), dQueue);
		}
	}
	else {
		//非空发送队列数大于7，生成7个0到size-1的不同的随机数，模拟随机选择7个
		vector<int> a;
		getRandNum(a, communicationNode.size(), CHANNEL_COUNT);
		iter = communicationNode.begin();
		int j = 0;
		for (int i = 0; i < CHANNEL_COUNT; i++)
		{
			for (int k = 0; k < a[i] && iter != communicationNode.end(); k++) {//循环直到随机数生成的那个发送队列 其余队列都设置成回退
				(*(*iter).getSendQueue()).setBackOff(true);
				iter++;
			}
			if (iter != communicationNode.end()) {//选中的发送队列回退标志设置成否
				(*(*iter).getSendQueue()).setBackOff(false);
				//获取其队首的消息送入待送达队列
				sendToDelieveQueue(*(*(*iter).getSendQueue()).getMessage(), dQueue);
			}
		}
	}
}

//##################################################################################
//物理信道模拟功能，将sMsg进行物理信道相关模拟
//##################################################################################
void MessageTransferManagement::channelSimulation(SliceMessage &sMsg, int index, list<CommunicationNode> communicationNodes, Channel &channel, int simulation_count)
{
	channel.bitSimulation(sMsg);//误码模拟
	channel.delaySimulation(sMsg, index, communicationNodes, simulation_count);//时延模拟
}

//##################################################################################
//获取所有非空且当前时隙不退避的发送队列，将其对应的通信节点存储在list中，最后返回list
//##################################################################################
list<CommunicationNode>* MessageTransferManagement::getNonemptySendQueueList(list<CommunicationNode> communicationNodes)
{
	list<CommunicationNode> *resSendQueues = new list<CommunicationNode>;
	list<CommunicationNode>::iterator iter;
	for (iter = communicationNodes.begin(); iter != communicationNodes.end(); ++iter)
	{
		if (!(*(*iter).getSendQueue()).isEmpty() && (*(*iter).getSendQueue()).getBackOffCount() == 0)
		{
			(*resSendQueues).push_front((*iter));
		}
	}

	return resSendQueues;
}

//###################################################################################
//将当前消息送到待送达队列
//###################################################################################
bool MessageTransferManagement::sendToDelieveQueue(SliceMessage sMsg, DelieveQueue &dQueue)
{
	return dQueue.insertToDelieveQ(sMsg);
}

//###################################################################################
//获取所有接收节点
//根据分片消息携带的内容，判断当前消息的接收节点
//###################################################################################
std::list<CommunicationNode> MessageTransferManagement::getAllReceiveNodes(SliceMessage sMsg,list<CommunicationNode> communicationNodes)
{
	if (sMsg.getDestinationAddress() == -1)//广播
	{
		return communicationNodes;
	}
	else {
		list<CommunicationNode> recvNodes;
		if (getNode(communicationNodes, sMsg.getDestinationAddress()).getNodeID() != (numeric_limits<long>::min)()) {
			recvNodes.push_front(getNode(communicationNodes, sMsg.getDestinationAddress()));
		}
		return recvNodes;
	}
}

//###################################################################################
//将当前消息送到接收队列
//###################################################################################
void MessageTransferManagement::sendToReceiveQueue(SliceMessage sMsg, list<CommunicationNode> &communicationNodes)
{
	CommunicationNode recNode = getNode(communicationNodes, sMsg.getDestinationAddress());
	(*(recNode.getReceiveQueue())).addMessage(sMsg);
	//cout << "receiveQueue's message: " << (*(recNode.getReceiveQueue())).getReceiveQueue()->back().getSourceAddress() << " "
		//<< (*(recNode.getReceiveQueue())).getReceiveQueue()->back().getDestinationAddress() << endl;
}



//###################################################################################
//检查消息是否误码
//###################################################################################
bool MessageTransferManagement::checkErrorMessage(SliceMessage sMsg)
{
	return errorCheck(sMsg);
}

//###################################################################################
//所有发送队列的退避处理
//###################################################################################
void MessageTransferManagement::backOffAll(list<CommunicationNode> &communicationNode, Protocol &protocol)
{
	list<CommunicationNode>::iterator iter;
	for (iter = communicationNode.begin(); iter != communicationNode.end(); iter++) {
		protocol.backOffAQueue(*iter);
	}
}
