#include "DeviceHandler.h"

// 声明全局设备对象.
extern PDEVICE_OBJECT g_pDeviceObject;

DeviceHandler::DeviceHandler():m_inputDataLen(0), m_inputData{0}
{
}

NTSTATUS DeviceHandler::create(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
	// 获取当前irp堆栈
	PIO_STACK_LOCATION irpsp = IoGetCurrentIrpStackLocation(pIrp);

	// 如果请求不是发给当前对象返回错误.
	if (pDeviceObject != g_pDeviceObject) {
		DbgPrint("Create:DeviceObject is not equal current device object.\r\n");
		return error(pIrp, STATUS_INVALID_PARAMETER);
	}
	return success(pIrp);
}

NTSTATUS DeviceHandler::close(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
	// 获取当前irp堆栈
	PIO_STACK_LOCATION irpsp = IoGetCurrentIrpStackLocation(pIrp);
	// 通过 do while 形式，打到try catch 效果.

	// 如果请求不是发给当前对象返回错误.
	if (pDeviceObject != g_pDeviceObject) {
		DbgPrint("Close:DeviceObject is not equal current device object.\r\n");
		return error(pIrp, STATUS_INVALID_PARAMETER);
	}
	return success(pIrp);
}

NTSTATUS DeviceHandler::deviceControl(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
	NTSTATUS status = STATUS_SUCCESS;
	// 此参数为 DeviceIoControl 中返回 lpBytesReturned 的指针.
	pIrp->IoStatus.Information = 0;
	// 获取当前设备堆栈.
	PIO_STACK_LOCATION irpsp = IoGetCurrentIrpStackLocation(pIrp);

	if (pDeviceObject != g_pDeviceObject) {
		return error(pIrp, STATUS_FAIL_CHECK);
	}

	// 获取缓冲区地址
	PVOID buff = pIrp->AssociatedIrp.SystemBuffer;
	// 获取输入缓冲区的长度
	ULONG inputBuffSize = irpsp->Parameters.DeviceIoControl.InputBufferLength;
	// 获取输出缓冲区长度
	ULONG outputBuffSize = irpsp->Parameters.DeviceIoControl.OutputBufferLength;


	switch (irpsp->Parameters.DeviceIoControl.IoControlCode)
	{
	case SEND_STR: // 向驱动写入数据.
		ASSERT(buff != NULL);
		ASSERT(inputBuffSize > 0);
		ASSERT(outputBuffSize == 0);
		return writeData(pDeviceObject, pIrp, inputBuffSize, outputBuffSize, buff);

	case RECV_STR: // 读取驱动返回数据.
		ASSERT(buff != NULL);
		ASSERT(inputBuffSize > 0);
		ASSERT(outputBuffSize > 0);
		return readData(pDeviceObject, pIrp, inputBuffSize, outputBuffSize, buff);

	default:
		// 到这里的请求都是错误请求.
		return error(pIrp, STATUS_INVALID_PARAMETER);
	}
}

void DeviceHandler::test()
{
	DbgPrint("[DH]DeviceHandler test() success.\r\n");
}

// 写入到驱动的数据处理
NTSTATUS DeviceHandler::writeData(PDEVICE_OBJECT pDeviceObject, PIRP pIrp, ULONG iBuffSize, ULONG oBuffSize, PVOID buff)
{
	if (m_inputDataLen + iBuffSize >= sizeof(m_inputData)) {
		return error(pIrp, STATUS_DATA_CHECKSUM_ERROR);
	}
	// 设置返回写入的数据长度.
	pIrp->IoStatus.Information = iBuffSize;
	memcpy(m_inputData + m_inputDataLen, buff, iBuffSize);
	m_inputDataLen += iBuffSize;
	// 判断最后一个字符是不是 '\0'
	if (m_inputData[m_inputDataLen - 1] != '\0') {
		m_inputData[m_inputDataLen] = '\0';
	}
	DbgPrint("WriteData Len:%d, Data:%s \r\n", iBuffSize, buff, iBuffSize);
	return success(pIrp, STATUS_SUCCESS, iBuffSize);
}

// 返回给用户的数据的处理.
NTSTATUS DeviceHandler::readData(PDEVICE_OBJECT pDeviceObject, PIRP pIrp, ULONG iBuffSize, ULONG oBuffSize, PVOID buff)
{	
	// 从写入缓冲区中获取请求的参数
	if (iBuffSize < sizeof(ParamsData)) {
		// 返回错误的请求参数.
		return error(pIrp, STATUS_INVALID_PARAMETER);
	}
	ULONG pos = PParamsData(buff)->pos;
	ULONG len = PParamsData(buff)->len;
	if (pos >= m_inputDataLen) {
		// 位置有问题，超过了数据实际存储有效位置.
		return error(pIrp, STATUS_INVALID_PARAMETER_1);
	}
	// 实际本次写入输出缓冲区的长度.
	int writeSize = 0;
	// 如果请求的数据长度在有效数据之内
	if (m_inputDataLen - pos >= len) {
		writeSize = len;
	}
	else {
		writeSize = m_inputDataLen - pos;
	}
	// 写入长度为缓冲区或者数据长度中较小的一个.
	writeSize = min(writeSize, oBuffSize);
	// 将数据复制到缓冲区中.如果一次无法全部返回，设置IoStatus.Status为STATUS_BUFFER_OVERFLOW，表示缓冲区不足。
	memcpy(buff, m_inputData + pos, writeSize);
	DbgPrint("ReadData Len:%d, Data:%s\r\n", writeSize, m_inputData + pos);
	// 判断是否一次可以写完请求数据.
	if (writeSize < oBuffSize) {
		return success(pIrp, STATUS_SUCCESS, writeSize);
	}
	else {
		// 表明仍有数据未写入.
		return success(pIrp, STATUS_BUFFER_OVERFLOW, writeSize);
	}
}

NTSTATUS DeviceHandler::error(PIRP pIrp, NTSTATUS status)
{
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS DeviceHandler::success(PIRP pIrp, NTSTATUS status, ULONG_PTR information)
{
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = information;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}
