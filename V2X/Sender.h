#pragma once
#ifndef SENDER_H
#define SENDER_H
#include <list>
#include "CommunicationNode.h"
#include "Protocol.h"
#include "Message.h"
#include "SliceMessage.h"
#include "SendQueue.h"

using namespace std;

/*
 *消息发送过程
*/
class Sender {
public:
	list<CommunicationNode> *oneCluster;//簇对象
	Protocol *protocol;//协议栈对象

public:
	Sender();
	Sender(list<CommunicationNode> &lc, Protocol &p);
	~Sender();
	void send();//控制流程

	list<SliceMessage> * sendToProtocol(Protocol &p, list<Message> &lm);
	void sendToSQueue(SendQueue &sq, list<SliceMessage> &lsm);//发送到发送队列
};

#endif