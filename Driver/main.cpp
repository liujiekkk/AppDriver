#include <ntifs.h>
#include "DeviceHandler.h"

#define __DBGPRINT__ "[AppDriver]"

// 定义设备名称
#define DEVICE_NAME L"\\Device\\DriverAppDemo"
// 定义设备连接符号.
#define DEVICE_SYMBOL_LINK L"\\??\\DeviceCustom_123456"

// 全局设备对象.
PDEVICE_OBJECT g_pDeviceObject = NULL;

typedef void (*IRP_DISPATCH_FUNC)(PDEVICE_OBJECT obj, PIRP irp);
// 设备操作对象.
typedef DeviceHandler* PDeviceHandler;

PDeviceHandler g_handler = nullptr;

// 定义设备扩展类型
typedef struct _DeviceExt {

}DEVICE_EXT, *PDEVICE_EXT;

// 卸载函数
VOID Unload()
{
	// 删除设备对象和符号链接
	UNICODE_STRING cdoSymbol = RTL_CONSTANT_STRING(DEVICE_SYMBOL_LINK);
	ASSERT(g_pDeviceObject != NULL);
	// 删除设备符号链接
	IoDeleteSymbolicLink(&cdoSymbol);
	// 删除设备对象.
	IoDeleteDevice(g_pDeviceObject);

	// 释放 DeviceHandler 对象.
	if (g_handler) {
		delete g_handler;
	}

	DbgPrint(__DBGPRINT__"Unload success.\r\n");
}

// 驱动程序加载入口程序.
EXTERN_C NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
	NTSTATUS status = STATUS_SUCCESS;
	pDriverObject->DriverUnload = (PDRIVER_UNLOAD)Unload;

	// 设备名称
	UNICODE_STRING deviceName = RTL_CONSTANT_STRING(DEVICE_NAME);

	// 创建一个设备对象.
	status = IoCreateDevice(pDriverObject, sizeof(DEVICE_EXT), &deviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &g_pDeviceObject);
	if (!NT_SUCCESS(status)) {
		DbgPrint(__DBGPRINT__"Create a device object failed.\r\n");
		return status;
	}
	// 创建一个设备符号链接.
	UNICODE_STRING symbolLink = RTL_CONSTANT_STRING(DEVICE_SYMBOL_LINK);
	status = IoCreateSymbolicLink(&symbolLink, &deviceName);
	if (!NT_SUCCESS(status)) {
		DbgPrint(__DBGPRINT__"Create a device symbol link failed.\r\n");
		// 删除设备对象.
		IoDeleteDevice(g_pDeviceObject);
		return status;
	}

	// 创建 DeviceHandler 对象.
	g_handler = new DeviceHandler();

	IRP_DISPATCH_FUNC funcCreate = [](PDEVICE_OBJECT obj, PIRP irp) {
		g_handler->create(obj, irp);
	};
	IRP_DISPATCH_FUNC funcClose = [](PDEVICE_OBJECT obj, PIRP irp) {
		g_handler->close(obj, irp);
	};
	IRP_DISPATCH_FUNC funcDeviceControl = [](PDEVICE_OBJECT obj, PIRP irp) {
		g_handler->deviceControl(obj, irp);
	};

	// 输出输出请求处理回调函数设置.
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = (PDRIVER_DISPATCH)funcCreate;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = (PDRIVER_DISPATCH)funcClose;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = (PDRIVER_DISPATCH)funcDeviceControl;

	DbgPrint(__DBGPRINT__"DriverEntry run success.\r\n");

	return status;
}
