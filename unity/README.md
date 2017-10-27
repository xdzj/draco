Description
===========

This folder contain resources for using Draco within Unity development.
Currently we support two types of usages:
* Import Draco compressed mesh as assets during design time.
* Load/decode Draco files in runtime.

Prerequisite
============

To start, you need to have the Draco unity plugin. You can either use the
prebuilt libraries provided in this folder or build from source.
Note that the plugin library for different platforms has different file extension.

| Platform | Library name |
| -------- | ------------ |
| Mac OS | dracodec_unity.bundle |
| Android | libdracodec_unity.so |
| Windows | TODO libdracodec_unity.dll |

Prebuilt Library
----------------

We have built library for several platforms:

| Platform | Tested Environment |
| -------- | ------------------ |
| .bundle  | macOS Sierra + Xcode 8.3.3 |
| armeabi-v7a | Android 8.1.0 |
| .dll | Win10 + Visual Studio 2017 |

Build From Source
-----------------

Mac OS X
--------
On Mac OS X, run the following command to generate Xcode projects:

~~~~~ bash
$ cmake path/to/draco -G Xcode -DBUILD_UNITY_PLUGIN=ON
~~~~~

Then open the project use Xcode and build.
You should be able to find the library under:
~~~~
path/to/build/Debug(or Release)/dracodec_unity.bundle
~~~~

Android
-------

You should first follow the [Android Studio Project Integration](../README.md) to build Draco within an Android project. Then, to build the plugin for Unity, just add the following flag to build.gradle.
~~~~
cppFlags "-DBUILD_UNITY_PLUGIN"
~~~~

You should be able to find the plugin library under:
~~~~
path/to/your/project/build/intermediates/cmake/debug(or release)/obj/your_platform/libdracodec_unity.so
~~~~

Windows
-------


Import Compressed Draco Assets
==============================

In this section we will describe how to import Draco files (.drc) to Unity as
other 3D formats, e.g. obj, fbx.

Load Draco Assets in Runtime
============================
In this section we will describe how to load Draco files during runtime.
