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
#ifdef BUILD_UNITY_PLUGIN

#include "draco/compression/config/compression_shared.h"
#include "draco/compression/config/decoder_options.h"
#include "draco/compression/decode.h"
#include "draco/core/decoder_buffer.h"
#include "draco/core/statusor.h"
#include "draco/mesh/mesh.h"

namespace draco {
/*
int TestCShapDLLMagicNumber() { return 123456; }

int DecodeBufferToMesh(char *data, unsigned int length) {
	draco::DecoderBuffer buffer;
	buffer.Init(data, length);
	auto type_statusor = draco::Decoder::GetEncodedGeometryType(&buffer);
	if (!type_statusor.ok()) {
		return -1;
	}
	const draco::EncodedGeometryType geom_type = type_statusor.value();
	if (geom_type != draco::TRIANGULAR_MESH) {
		return -1;
	}
	draco::Decoder decoder;
	std::unique_ptr<draco::PointCloud> pc;
	draco::Mesh *mesh = nullptr;
	auto statusor = decoder.DecodeMeshFromBuffer(&buffer);
	if (!statusor.ok()) {
		return -1;
	}
	std::unique_ptr<draco::Mesh> in_mesh = std::move(statusor).value();
	const int num_faces = in_mesh->num_faces();

	if (in_mesh) {
		mesh = in_mesh.get();
		pc = std::move(in_mesh);
	}
	if (pc == nullptr) {
		return -1;
	}

	return num_faces;
}
*/
}  // namespace draco

#endif // BUILD_UNITY_PLUGIN