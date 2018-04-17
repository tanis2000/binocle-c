//
//  Binocle
//  Copyright(C)2015-2017 Valerio Santinelli
//

#if defined(__IPHONEOS__) || defined(__MACOSX__)
#include "PlayerPrefs.hpp"
#import <string>
#import <Foundation/Foundation.h>

using namespace std;
namespace Binocle
{
    void PlayerPrefs::Synchronize()
    {
        [[NSUserDefaults standardUserDefaults]synchronize];
    }
    
    
    //bool
    bool PlayerPrefs::GetBoolForKey(const char* pKey, bool defaultValue)
    {
        bool ret = defaultValue;
        NSNumber *value = [[NSUserDefaults standardUserDefaults] objectForKey:[NSString stringWithUTF8String:pKey]];
        if (value)
        {
            ret = [value boolValue];
        }
        return ret;
    }
    void PlayerPrefs::SetBoolForKey(const char* pKey, bool value)
    {
        [[NSUserDefaults standardUserDefaults]setBool:value forKey:[NSString stringWithUTF8String:pKey]];
    }
    
    //int
    static int GetIntForKey(const char* pKey, int defaultValue)
    {
        int ret = defaultValue;
        NSNumber *value = [[NSUserDefaults standardUserDefaults] objectForKey:[NSString stringWithUTF8String:pKey]];
        if (value)
        {
            ret = [value intValue];
        }
        return ret;
    }
    static void SetIntForKey(const char* pKey, int value)
    {
        [[NSUserDefaults standardUserDefaults]setInteger:value forKey:[NSString stringWithUTF8String:pKey]];
    }
    
    
    //float
    static float GetFloatForKey(const char* pKey, float defaultValue)
    {
        float ret = defaultValue;
        NSNumber *value = [[NSUserDefaults standardUserDefaults] objectForKey:[NSString stringWithUTF8String:pKey]];
        if (value)
        {
            ret = [value floatValue];
        }
        return ret;
    }
    static void SetFloatForKey(const char* pKey, float value)
    {
        [[NSUserDefaults standardUserDefaults]setFloat:value forKey:[NSString stringWithUTF8String:pKey]];
    }
    
    //double
    static double GetDoubleForKey(const char* pKey, double defaultValue)
    {
        double ret = defaultValue;
        NSNumber *value = [[NSUserDefaults standardUserDefaults] objectForKey:[NSString stringWithUTF8String:pKey]];
        if (value)
        {
            ret = [value doubleValue];
        }
        return ret;
    }
    static void SetDoubleForKey(const char* pKey, double value)
    {
        [[NSUserDefaults standardUserDefaults]setDouble:value forKey:[NSString stringWithUTF8String:pKey]];
    }
    
    //string
    std::string GetStringForKey(const char* pKey, const std::string & defaultValue)
    {
        NSString *str = [[NSUserDefaults standardUserDefaults] stringForKey:[NSString stringWithUTF8String:pKey]];
        if (! str)
        {
            return defaultValue;
        }
        else
        {
            return [str UTF8String];
        }
    }
    void SetStringForKey(const char* pKey, const std::string & value)
    {
        [[NSUserDefaults standardUserDefaults] setObject:[NSString stringWithUTF8String:value.c_str()] forKey:[NSString stringWithUTF8String:pKey]];
    }
}
#endif