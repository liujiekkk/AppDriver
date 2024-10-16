#include <iostream>
#include <Windows.h>


#define DEVICE_SYMBOL_LINK L"\\\\.\\DeviceCustom_123456"

// 向驱动设备发送字符串
#define SEND_STR (ULONG)CTL_CODE(FILE_DEVICE_UNKNOWN, 0x911, METHOD_BUFFERED, FILE_WRITE_DATA)
// 从驱动设备读取字符串
#define RECV_STR (ULONG)CTL_CODE(FILE_DEVICE_UNKNOWN, 0x912, METHOD_BUFFERED, FILE_READ_DATA)

#define BUFF_SIZE 10


// 用来请求驱动内部数据时候的参数.
typedef struct {
	ULONG pos; // 位置.
	ULONG len; // 长度.
} ParamsData, * PParamsData;

// 从指定设备读取目标指定长度数据，返回实际读取的长度.
static DWORD readFromDevice(HANDLE hDevice, PVOID buff, DWORD size, PVOID params = NULL)
{
	DWORD retLen = 0;
	// 设置缓冲区大小，防止写入超长数据.
	DWORD buffSize = min(BUFF_SIZE, size);
	if (params != NULL) {
		if (!DeviceIoControl(hDevice, RECV_STR, params, sizeof(ParamsData), buff, buffSize, &retLen, 0)) {
			return -1;
		}
	}
	else {
		if (!DeviceIoControl(hDevice, RECV_STR, NULL, 0, buff, buffSize, &retLen, 0)) {
			return -1;
		}
	}
	return retLen;
}
// 向指定设备写入指定长度的数据，返回实际写入的长度.
static DWORD writeToDevice(HANDLE hDevice, PVOID buff, DWORD size)
{
	DWORD retLen = 0;
	// 设置缓冲区大小，防止写入超长数据.
	DWORD buffSize = min(BUFF_SIZE, size);
	if (!DeviceIoControl(hDevice, SEND_STR, buff, buffSize, NULL, 0, &retLen, 0)) {
		return -1;
	}
	return retLen;
}


void testWriteData(HANDLE device)
{
	// 向内核驱动设备写入数据一次的实际长度.
	DWORD retLen = 0, ready = 0;
	const char* msg = "Hello driver, this is a string from app.";
	// 计算发送数据的长度 left.
	DWORD dataLen = DWORD(strlen(msg) + 1);
	// 可能数据一次发送不完，需要反复发送才能成功.
	while (dataLen > ready) {
		DWORD len = writeToDevice(device, LPVOID(msg + ready), dataLen - ready);
		if (len) {
			ready += len;
		}
		else {
			printf("wirte data failed.\r\n");
			break;
		}
	}
	printf("Send Message success.\r\n");
}

void testReadData(HANDLE device)
{
	// 向内核驱动设备写入数据一次的实际长度.
	DWORD retLen = 0, ready = 0;
	// 存储读取的数据.
	char msg[1024] = { 0 };

	ParamsData param = {10, 10};

	// 可能数据一次发送不完，需要反复发送才能成功.
	while (param.len > ready) {
		DWORD len = readFromDevice(device, LPVOID(msg + ready), param.len - ready, &param);
		if (len) {
			ready += len;
		}
		else {
			printf("read data failed.\r\n");
			break;
		}
	}
	printf("Read Message success.\r\n");
	printf("DATA: %s", msg);
}

int main(int argc, TCHAR* argv[]) {

	HANDLE device = NULL;
	// 打开设备符号链接获取设备句柄.
	device = CreateFile(DEVICE_SYMBOL_LINK, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, 0);
	if (device == INVALID_HANDLE_VALUE) {
		printf("open device failed.\r\n");
		return -1;
	}
	printf("open device success.\r\n");

	// 测试向内核驱动写入数据.
	testWriteData(device);

	// 读取内核驱动设备返回的数据.
	testReadData(device);

	// 关闭设备.
	CloseHandle(device);
	return 0;
}
