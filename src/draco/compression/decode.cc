// Copyright 2016 The Draco Authors.
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
#include "draco/compression/decode.h"

#include <cstdlib>

#include "draco/compression/config/compression_shared.h"

#ifdef DRACO_MESH_COMPRESSION_SUPPORTED
#include "draco/compression/mesh/mesh_edgebreaker_decoder.h"
#include "draco/compression/mesh/mesh_sequential_decoder.h"
#endif

#ifdef DRACO_POINT_CLOUD_COMPRESSION_SUPPORTED
#include "draco/compression/point_cloud/point_cloud_kd_tree_decoder.h"
#include "draco/compression/point_cloud/point_cloud_sequential_decoder.h"
#endif

namespace draco {

#ifdef DRACO_POINT_CLOUD_COMPRESSION_SUPPORTED
StatusOr<std::unique_ptr<PointCloudDecoder>> CreatePointCloudDecoder(
    int8_t method) {
  if (method == POINT_CLOUD_SEQUENTIAL_ENCODING) {
    return std::unique_ptr<PointCloudDecoder>(
        new PointCloudSequentialDecoder());
  } else if (method == POINT_CLOUD_KD_TREE_ENCODING) {
    return std::unique_ptr<PointCloudDecoder>(new PointCloudKdTreeDecoder());
  }
  return Status(Status::ERROR, "Unsupported encoding method.");
}
#endif

#ifdef DRACO_MESH_COMPRESSION_SUPPORTED
StatusOr<std::unique_ptr<MeshDecoder>> CreateMeshDecoder(uint8_t method) {
  if (method == MESH_SEQUENTIAL_ENCODING) {
    return std::unique_ptr<MeshDecoder>(new MeshSequentialDecoder());
  } else if (method == MESH_EDGEBREAKER_ENCODING) {
    return std::unique_ptr<MeshDecoder>(new MeshEdgeBreakerDecoder());
  }
  return Status(Status::ERROR, "Unsupported encoding method.");
}
#endif

StatusOr<EncodedGeometryType> Decoder::GetEncodedGeometryType(
    DecoderBuffer *in_buffer) {
  DecoderBuffer temp_buffer(*in_buffer);
  DracoHeader header;
  DRACO_RETURN_IF_ERROR(PointCloudDecoder::DecodeHeader(&temp_buffer, &header))
  return static_cast<EncodedGeometryType>(header.encoder_type);
}

StatusOr<std::unique_ptr<PointCloud>> Decoder::DecodePointCloudFromBuffer(
    DecoderBuffer *in_buffer) {
  DRACO_ASSIGN_OR_RETURN(EncodedGeometryType type,
                         GetEncodedGeometryType(in_buffer))
  if (type == POINT_CLOUD) {
#ifdef DRACO_POINT_CLOUD_COMPRESSION_SUPPORTED
    std::unique_ptr<PointCloud> point_cloud(new PointCloud());
    DRACO_RETURN_IF_ERROR(DecodeBufferToGeometry(in_buffer, point_cloud.get()))
    return std::move(point_cloud);
#endif
  } else if (type == TRIANGULAR_MESH) {
#ifdef DRACO_MESH_COMPRESSION_SUPPORTED
    std::unique_ptr<Mesh> mesh(new Mesh());
    DRACO_RETURN_IF_ERROR(DecodeBufferToGeometry(in_buffer, mesh.get()))
    return static_cast<std::unique_ptr<PointCloud>>(std::move(mesh));
#endif
  }
  return Status(Status::ERROR, "Unsupported geometry type.");
}

StatusOr<std::unique_ptr<Mesh>> Decoder::DecodeMeshFromBuffer(
    DecoderBuffer *in_buffer) {
  std::unique_ptr<Mesh> mesh(new Mesh());
  DRACO_RETURN_IF_ERROR(DecodeBufferToGeometry(in_buffer, mesh.get()))
  return std::move(mesh);
}

Status Decoder::DecodeBufferToGeometry(DecoderBuffer *in_buffer,
                                       PointCloud *out_geometry) {
#ifdef DRACO_POINT_CLOUD_COMPRESSION_SUPPORTED
  DecoderBuffer temp_buffer(*in_buffer);
  DracoHeader header;
  DRACO_RETURN_IF_ERROR(PointCloudDecoder::DecodeHeader(&temp_buffer, &header))
  if (header.encoder_type != POINT_CLOUD) {
    return Status(Status::ERROR, "Input is not a point cloud.");
  }
  DRACO_ASSIGN_OR_RETURN(std::unique_ptr<PointCloudDecoder> decoder,
                         CreatePointCloudDecoder(header.encoder_method))

  DRACO_RETURN_IF_ERROR(decoder->Decode(options_, in_buffer, out_geometry))
  return OkStatus();
#else
  return Status(Status::ERROR, "Unsupported geometry type.");
#endif
}

Status Decoder::DecodeBufferToGeometry(DecoderBuffer *in_buffer,
                                       Mesh *out_geometry) {
#ifdef DRACO_MESH_COMPRESSION_SUPPORTED
  DecoderBuffer temp_buffer(*in_buffer);
  DracoHeader header;
  DRACO_RETURN_IF_ERROR(PointCloudDecoder::DecodeHeader(&temp_buffer, &header))
  if (header.encoder_type != TRIANGULAR_MESH) {
    return Status(Status::ERROR, "Input is not a mesh.");
  }
  DRACO_ASSIGN_OR_RETURN(std::unique_ptr<MeshDecoder> decoder,
                         CreateMeshDecoder(header.encoder_method))

  DRACO_RETURN_IF_ERROR(decoder->Decode(options_, in_buffer, out_geometry))
  return OkStatus();
#else
  return Status(Status::ERROR, "Unsupported geometry type.");
#endif
}

void Decoder::SetSkipAttributeTransform(GeometryAttribute::Type att_type) {
  options_.SetAttributeBool(att_type, "skip_attribute_transform", true);
}

int TestDecodingAndroid(char *data, unsigned int length) {
  draco::DecoderBuffer buffer;
  buffer.Init(data, length);
  auto type_statusor = draco::Decoder::GetEncodedGeometryType(&buffer);
  if (!type_statusor.ok()) {
    return -1;
  }
  const draco::EncodedGeometryType geom_type = type_statusor.value();
  if (geom_type != draco::TRIANGULAR_MESH) {
    return -2;
  }
  draco::Decoder decoder_old;
  std::unique_ptr<draco::PointCloud> pc;

  DracoHeader header;
  auto header_statusor = PointCloudDecoder::DecodeHeader(&buffer, &header);
  if (!type_statusor.ok()) {
    return -3;
  }
  if (header.encoder_type != TRIANGULAR_MESH) {
    return -4;
  }
  auto decoder_statusor = CreateMeshDecoder(header.encoder_method);
  if (!decoder_statusor.ok()) {
    return -5;
  }
  std::unique_ptr<MeshDecoder> decoder = std::move(decoder_statusor).value();
  DecoderOptions options;
  std::unique_ptr<Mesh> mesh(new Mesh());
  auto decode_statusor = decoder->Decode(options, &buffer, mesh.get());
  if (!decoder_statusor.ok()) {
    return -6;
  }

  /*
  auto statusor = decoder_old.DecodeMeshFromBuffer(&buffer);
  if (!statusor.ok()) {
      return -3;
  }
  std::unique_ptr<draco::Mesh> in_mesh = std::move(statusor).value();

  if (in_mesh) {
      mesh = in_mesh.get();
      pc = std::move(in_mesh);
  }
  if (pc == nullptr) {
      return -4;
  }
   */
  const int num_faces = mesh->num_faces();
  return num_faces;
}
int DecodeMeshForUnity(char *data, unsigned int length,
                       DracoToUnityMesh **tmp_mesh) {
  draco::DecoderBuffer buffer;
  buffer.Init(data, length);
  auto type_statusor = draco::Decoder::GetEncodedGeometryType(&buffer);
  if (!type_statusor.ok()) {
    return -1;
  }
  const draco::EncodedGeometryType geom_type = type_statusor.value();
  if (geom_type != draco::TRIANGULAR_MESH) {
    return -2;
  }

  draco::Decoder decoder;
  auto statusor = decoder.DecodeMeshFromBuffer(&buffer);
  if (!statusor.ok()) {
    return -3;
  }
  std::unique_ptr<draco::Mesh> in_mesh = std::move(statusor).value();

  *tmp_mesh = new DracoToUnityMesh();
  DracoToUnityMesh *unity_mesh = *tmp_mesh;
  unity_mesh->num_faces = in_mesh->num_faces();
  unity_mesh->num_vertices = in_mesh->num_points();

  unity_mesh->indices = new int[in_mesh->num_faces() * 3];
  for (draco::FaceIndex face_id(0); face_id < in_mesh->num_faces(); ++face_id) {
    const Mesh::Face &face = in_mesh->face(draco::FaceIndex(face_id));
    memcpy(unity_mesh->indices + face_id.value() * 3, face.data(),
           sizeof(int) * 3);
  }

  // std::unique_ptr<draco::PointCloud> pc;
  // pc = std::move(in_mesh);
  unity_mesh->position = new float[in_mesh->num_points() * 3];
  const auto pos_att =
      in_mesh->GetNamedAttribute(draco::GeometryAttribute::POSITION);
  std::array<float, 3> value;
  for (draco::PointIndex i(0); i < in_mesh->num_points(); ++i) {
    const draco::AttributeValueIndex val_index = pos_att->mapped_index(i);
    if (!pos_att->ConvertValue<float, 3>(val_index, &value[0])) return -8;
    memcpy(unity_mesh->position + i.value() * 3, value.data(),
           sizeof(float) * 3);
  }

  return in_mesh->num_faces();
}
int DecodeMeshForUnityDebug(char *data, unsigned int length,
                            DracoToUnityMesh **tmp_mesh) {
  draco::DecoderBuffer buffer;
  buffer.Init(data, length);
  auto type_statusor = draco::Decoder::GetEncodedGeometryType(&buffer);
  if (!type_statusor.ok()) {
    return -1;
  }
  const draco::EncodedGeometryType geom_type = type_statusor.value();
  if (geom_type != draco::TRIANGULAR_MESH) {
    return -2;
  }

  DecoderBuffer temp_buffer(buffer);
  DracoHeader header;
  auto header_statusor = PointCloudDecoder::DecodeHeader(&temp_buffer, &header);
  if (!type_statusor.ok()) {
    return -4;
  }
  if (header.encoder_type != TRIANGULAR_MESH) {
    return -5;
  }
  if (header.encoder_method != MESH_EDGEBREAKER_ENCODING) {
    return -8;
  }
  auto decoder_statusor = CreateMeshDecoder(header.encoder_method);
  if (!decoder_statusor.ok()) {
    return -5;
  }
  std::unique_ptr<MeshDecoder> decoder_2 =
      std::move(std::move(decoder_statusor).value());

  DecoderOptions options;
  std::unique_ptr<draco::Mesh> in_mesh(new draco::Mesh());
  if (in_mesh == nullptr) return -9;

  auto decode_statusor = decoder_2->Decode(options, &buffer, in_mesh.get());
  if (!decode_statusor.ok()) return -7;

  /*
   draco::Status decode_status = decoder_2->Decode(options, tmp_buffer,
   in_mesh.get()); if (decode_status.code() != draco::Status::OK) { return
   decode_status.code();
   }
   */

  *tmp_mesh = new DracoToUnityMesh();
  DracoToUnityMesh *unity_mesh = *tmp_mesh;
  unity_mesh->num_faces = in_mesh->num_faces();
  unity_mesh->num_vertices = in_mesh->num_points();

  unity_mesh->indices = new int[in_mesh->num_faces() * 3];
  for (draco::FaceIndex face_id(0); face_id < in_mesh->num_faces(); ++face_id) {
    const Mesh::Face &face = in_mesh->face(draco::FaceIndex(face_id));
    memcpy(unity_mesh->indices + face_id.value() * 3, face.data(),
           sizeof(int) * 3);
  }

  // std::unique_ptr<draco::PointCloud> pc;
  // pc = std::move(in_mesh);
  unity_mesh->position = new float[in_mesh->num_points() * 3];
  const auto pos_att =
      in_mesh->GetNamedAttribute(draco::GeometryAttribute::POSITION);
  std::array<float, 3> value;
  for (draco::PointIndex i(0); i < in_mesh->num_points(); ++i) {
    const draco::AttributeValueIndex val_index = pos_att->mapped_index(i);
    if (!pos_att->ConvertValue<float, 3>(val_index, &value[0])) return -8;
    memcpy(unity_mesh->position + i.value() * 3, value.data(),
           sizeof(float) * 3);
  }

  return in_mesh->num_faces();
}

}  // namespace draco
