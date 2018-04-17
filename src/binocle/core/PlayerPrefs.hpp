//
//  Binocle
//  Copyright(C)2015-2017 Valerio Santinelli
//

#ifndef PlayerPrefs_hpp
#define PlayerPrefs_hpp

#include <stdio.h>
#include <string>

namespace Binocle {
  class PlayerPrefs {
  public:
    static void Synchronize();

    //bool
    static bool GetBoolForKey(const char *pKey, bool defaultValue);

    static void SetBoolForKey(const char *pKey, bool value);

    //int
    static int GetIntForKey(const char *pKey, int defaultValue);

    static void SetIntForKey(const char *pKey, int value);

    //float
    static int GetFloatForKey(const char *pKey, float defaultValue);

    static void SetFloatForKey(const char *pKey, float value);

    //double
    static int GetDoubleForKey(const char *pKey, double defaultValue);

    static void SetDoubleForKey(const char *pKey, double value);

    //string
    static std::string GetStringForKey(const char *pKey, const std::string &defaultValue);

    static void SetStringForKey(const char *pKey, const std::string &value);


  };
}

#endif /* PlayerPrefs_hpp */
