#pragma once
#include <string>
using namespace std;

/*
 *原始消息类
*/
class Message
{
public://公用接口
	/*******构造函数*********/
	Message();
	Message(long msgID, long sa, long da, int sz);
	//析构函数
	~Message();

	/**********信息函数：获取消息的相关信息*********/
	//获取消息ID
	long getMessageID();
	//获取源地址
	long getSourceAddress();
	//获取目的地址
	long getDestinationAddress();
	//获取消息大小
	int getSize();
	//获取消息类型
	int getMessageType();
	//获取传输类型
	int getTransmissionType();
	//获取消息时延
	float getDelay();
	//获取是否传输完毕标志
	bool getTransFinish();

	//设置消息ID
	void setMessageID(long id);
	//设置源地址
	void setSourceAddress(long sourceAddress);
	//设置目的地址
	void setDestinationAddress(long destinationAddress);
	//设置消息大小
	void setSize(int size);
	//设置消息类型
	void setMessageType(int messageType);
	//设置传输类型
	void setTransmissionType(int transmissionType);
	//设置消息时延
	void setDelay(float delay);
	//设置是否传输完成标志
	void setTransFinish(bool trans);

	//消息转换成Json串
	std::string toJson();

private://消息内部变量
	long MessageID;            //消息ID
	long SourceAddress;        //消息的源地址
	long DestinationAddress;   //消息的目的地址
	int Size;                  //消息大小
	int MessageType;           //消息类型
	int TransmissionType;      //消息的传输类型
	float Delay = 0;           //时延
	bool TransFinish = false;	//是否完整传输完成 只在接收端有用
};