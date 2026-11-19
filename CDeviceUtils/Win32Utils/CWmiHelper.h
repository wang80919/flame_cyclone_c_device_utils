#pragma once
#include <Wbemidl.h>
#include <tchar.h>
#include <atlcomcli.h>
#include <string>
#include <vector>
#include <set>
#include <functional>
#include <cstdint>

#define WMI_GET_VALUE(_obj, _out, _name) _obj.GetValue(_T(#_name), _out._name)

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

// ============================================================================
// 常用查询参考类
// ============================================================================
// 操作系统类
// https://learn.microsoft.com/zh-cn/windows/win32/cimwin32prov/operating-system-classes
//
// 计算机系统硬件类
// https://learn.microsoft.com/zh-cn/windows/win32/cimwin32prov/computer-system-hardware-classes

// WMI 基础类型定义
using wmi_boolean = bool;
using wmi_uint8 = uint8_t;
using wmi_uint8_list = std::vector<wmi_uint8>;
using wmi_sint8 = int8_t;
using wmi_sint8_list = std::vector<wmi_sint8>;
using wmi_uint16 = uint16_t;
using wmi_uint16_list = std::vector<wmi_uint16>;
using wmi_sint16 = int16_t;
using wmi_sint16_list = std::vector<wmi_sint16>;
using wmi_uint32 = uint32_t;
using wmi_uint32_list = std::vector<wmi_uint32>;
using wmi_sint32 = int32_t;
using wmi_sint32_list = std::vector<wmi_sint32>;
using wmi_uint64 = uint64_t;
using wmi_uint64_list = std::vector<wmi_uint64>;
using wmi_sint64 = int64_t;
using wmi_sint64_list = std::vector<wmi_sint64>;
using wmi_string = _tstring;
using wmi_string_list = std::vector<wmi_string>;
using wmi_float = float_t;
using wmi_float_list = std::vector<wmi_float>;
using wmi_double = double_t;
using wmi_double_list = std::vector<wmi_double>;
using wmi_real32 = wmi_float;
using wmi_real32_list = wmi_float_list;
using wmi_real64 = wmi_double;
using wmi_real64_list = wmi_double_list;
using wmi_datetime = SYSTEMTIME;
using wmi_datetime_list = std::vector<wmi_datetime>;

// WMI 类辅助类
class CWmiClassHelper
{
public:
    explicit CWmiClassHelper(IWbemClassObject* pObj);

    // 获取属性类型
    CIMTYPE GetCimType(const _tstring& strName) const;
    VARTYPE GetVarType(const _tstring& strName) const;

    // 获取值
    wmi_boolean GetBoolean(const _tstring& strName, bool fTypeCheck = false) const;
    wmi_sint64 GetSInt(const _tstring& strName, bool fTypeCheck = false) const;
    wmi_uint64 GetUInt(const _tstring& strName, bool fTypeCheck = false) const;
    wmi_double GetFloat(const _tstring& strName, bool fTypeCheck = false) const;
    wmi_string GetString(const _tstring& strName, bool fTypeCheck = false) const;
    wmi_datetime GetDatetime(const _tstring& strName) const;
    bool GetValue(const _tstring& strName, wmi_boolean& val) const;
    bool GetValue(const _tstring& strName, wmi_sint8& val) const;
    bool GetValue(const _tstring& strName, wmi_uint8& val) const;
    bool GetValue(const _tstring& strName, wmi_sint16& val) const;
    bool GetValue(const _tstring& strName, wmi_uint16& val) const;
    bool GetValue(const _tstring& strName, wmi_sint32& val) const;
    bool GetValue(const _tstring& strName, wmi_uint32& val) const;
    bool GetValue(const _tstring& strName, wmi_sint64& val) const;
    bool GetValue(const _tstring& strName, wmi_uint64& val) const;
    bool GetValue(const _tstring& strName, wmi_float& val) const;
    bool GetValue(const _tstring& strName, wmi_double& val) const;
    bool GetValue(const _tstring& strName, wmi_string& val) const;
    bool GetValue(const _tstring& strName, wmi_datetime& val) const;
    bool GetValue(const _tstring& strName, wmi_sint8_list& valList) const;
    bool GetValue(const _tstring& strName, wmi_uint8_list& valList) const;
    bool GetValue(const _tstring& strName, wmi_sint16_list& valList) const;
    bool GetValue(const _tstring& strName, wmi_uint16_list& valList) const;
    bool GetValue(const _tstring& strName, wmi_sint32_list& valList) const;
    bool GetValue(const _tstring& strName, wmi_uint32_list& valList) const;
    bool GetValue(const _tstring& strName, wmi_sint64_list& valList) const;
    bool GetValue(const _tstring& strName, wmi_uint64_list& valList) const;
    bool GetValue(const _tstring& strName, wmi_float_list& valList) const;
    bool GetValue(const _tstring& strName, wmi_double_list& valList) const;
    bool GetValue(const _tstring& strName, wmi_string_list& valList) const;
    bool GetValue(const _tstring& strName, wmi_datetime_list& valList) const;

    // 获取系统属性
    wmi_string GetClass() const;
    wmi_string GetSuperClass() const;
    wmi_string GetNameSpace() const;
    wmi_string GetPath() const;
    wmi_string GetServer() const;
    wmi_string GetRelPath() const;
    wmi_sint32 GetPropertyCount() const;
    wmi_sint32 GetGenus() const;
    wmi_string GetDynasty() const;
    wmi_string GetDerivation() const;

    // 打印属性
    void PrintProperty() const;
    void PrintPropertySystemOnly() const;
    void PrintPropertyNonSystemOnly() const;
    void ConsolePrint(LPCTSTR pFormat, ...) const;

private:

    //
    // @brief:  获取值
    // @param:  strName             字段名
    // @param:  ct                  原始值类型
    // @param:  cb                  回调函数
    // @ret:    bool                操作是否成功
    bool _GetValue(
        const _tstring& strName, 
        CIMTYPE ct, 
        std::function<void(const CComVariant& vtProp)> cb
    ) const;

    //
    // @brief:  获取值列表(一维数组)
    // @param:  strName             字段名
    // @param:  ct                  元素原始值类型
    // @param:  vt                  元素值类型
    // @param:  cb                  回调函数
    // @ret:    bool                操作是否成功
    bool _GetValueList(
        const _tstring& strName, 
        CIMTYPE ct, 
        VARTYPE vt, 
        std::function<void(const void* pValue, UINT uElemsize, LONG count, VARTYPE vt)> cb
    ) const;

    // 打印
    void _PrintProperty(long lFlags) const;
    void _Print(const CComVariant& val) const;
    void _PrintList(const CComVariant& val) const;
    void _Print(const _tstring& strName) const;

private:
    IWbemClassObject* m_pObj;
};

// WMI 查询辅助类
class CWmiHelper
{
public:

    CWmiHelper();
    ~CWmiHelper();

    bool Initialize(_tstring strNamespace = _T(R"(ROOT\CIMV2)"));
    void Uninitialize();

    //
    // @brief:  查询指定类字段名集合
    // @param:  strClass            类名, 如: Win32_Bios
    // @ret:    std::set<_tstring>  字段名集合
    std::set<wmi_string> QueryClassNames(const _tstring& strClass) const;

    //
    // @brief:  查询
    // @param:  strWql              查询语句
    // @param:  cb                  结果回调
    // @ret:    bool                操作是否成功
    bool Query(
        const _tstring& strWql,
        std::function<void(const CWmiClassHelper& obj)> cb = nullptr
    ) const;

    //
    // @brief:  查询指定类属性
    // @param:  strClass            类名, 如: Win32_Bios
    // @param:  cb                  结果回调
    // @ret:    bool                操作是否成功
    bool QueryClass(
        const _tstring& strClass,
        std::function<void(const CWmiClassHelper& obj)> cb = nullptr
    ) const;

    //
    // @brief:  查询指定类属性(指定字段)
    // @param:  strClass            类名, 如: Win32_Bios
    // @param:  names               字段列表
    // @param:  cb                  结果回调
    // @ret:    bool                操作是否成功
    bool QueryClassWithNames(
        const _tstring& strClass,
        std::set<wmi_string> names = {},
        std::function<void(const CWmiClassHelper& obj)> cb = nullptr
    ) const;

    //
    // @brief:  查询指定类属性(排除字段)
    // @param:  strClass            类名, 如: Win32_Bios
    // @param:  names               排除字段集
    // @param:  cb                  结果回调
    // @ret:    bool                操作是否成功
    bool QueryClassWithoutNames(
        const _tstring& strClass,
        std::set<wmi_string> names = {},
        std::function<void(const CWmiClassHelper& obj)> cb = nullptr
    ) const;

private:
    bool m_bInit;                   // 初始化结果
    IWbemServices* m_pSvc;          // 客户端和提供程序使用的 WMI 服务
    IWbemLocator* m_pLoc;           // 指向特定主机上 WMI 的 IWbemServices 接口的初始命名空间指针
};
