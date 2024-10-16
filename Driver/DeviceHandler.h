#pragma once

#include "Base.h"
#include <ntifs.h>

// 向驱动设备发送字符串
#define SEND_STR (ULONG)CTL_CODE(FILE_DEVICE_UNKNOWN, 0x911, METHOD_BUFFERED, FILE_WRITE_DATA)
// 从驱动设备读取字符串
#define RECV_STR (ULONG)CTL_CODE(FILE_DEVICE_UNKNOWN, 0x912, METHOD_BUFFERED, FILE_READ_DATA)

// 用来请求驱动内部数据时候的参数.
typedef struct {
	ULONG pos; // 位置.
	ULONG len; // 长度.
} ParamsData, *PParamsData;

// 设备IRP请求处理类.
class DeviceHandler :public Base
{
private:

	// 输入数据长度.
	USHORT m_inputDataLen;

	CHAR m_inputData[1024];

public:

	DeviceHandler();

	// funciton for IRP_MJ_CREATE
	NTSTATUS create(PDEVICE_OBJECT, PIRP);

	// function for IRP_MJ_CLOSE
	NTSTATUS close(PDEVICE_OBJECT, PIRP);

	// function for IRP_MJ_DEVICE_CONTROL
	NTSTATUS deviceControl(PDEVICE_OBJECT, PIRP);

	void test();

protected:

	// 返回错误状态.
	NTSTATUS error(PIRP pIrp, NTSTATUS);

	// 返回成功状态
	NTSTATUS success(PIRP pIrp, NTSTATUS status = STATUS_SUCCESS, ULONG_PTR information = 0);

	NTSTATUS writeData(PDEVICE_OBJECT, PIRP, ULONG, ULONG, PVOID);

	NTSTATUS readData(PDEVICE_OBJECT, PIRP, ULONG, ULONG, PVOID);

};
