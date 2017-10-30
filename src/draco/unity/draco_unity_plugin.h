// Copyright 2017 The Draco Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#ifndef DRACO_UNITY_DRACO_UNITY_PLUGIN_H_
#define DRACO_UNITY_DRACO_UNITY_PLUGIN_H_

#include "draco/compression/config/compression_shared.h"
#include "draco/compression/decode.h"

//#ifdef BUILD_UNITY_PLUGIN

namespace draco {

#if defined(_WIN32)
    extern "C" __declspec(dllexport)
#else
    extern "C"
#endif
    {
        int TestUnityModule();
        
    }
    
#if defined(_WIN32)
    extern "C" __declspec(dllexport)
#else
    extern "C"
#endif
    {
        struct DracoToUnityMesh {
            int num_faces;
            int *indices;
            int num_vertices;
            
            float *position;
            float *normal;
            float *texcoord;
            float *color;
        };
        int DecodeMeshForUnity(char *data, unsigned int length,
                               DracoToUnityMesh **tmp_mesh);
        
        //int TestDecodingAndroid(char *data, unsigned int length);
    }  // extern "C"

    
}  // namespace draco

//#endif // BUILD_UNITY_PLUGIN

#endif // DRACO_UNITY_DRACO_UNITY_PLUGIN_H_
