using System.Collections;
using System.Collections.Generic;
using UnityEngine;

using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

public unsafe class DracoMeshLoader
{
	// Must stay the order to be consistent with C++ interface.
	[ StructLayout( LayoutKind.Sequential )]
	private struct DracoToUnityMesh
	{
		public int num_faces;
		public IntPtr indices;
		public int num_vertices;
		public IntPtr position;
		//public int num_normal;
		public float[] normal;
		//public int num_color;
		public float[] color;
		public float[] texcoord;
	}

	[DllImport ("dracodec_unity")]
	private static extern int DecodeMeshForUnity (byte[] buffer, int length, DracoToUnityMesh **tmp_mesh);

	[DllImport ("dracodec_unity")]
	private static extern int TestDecodingAndroid (byte[] buffer, int length);

	private float ReadFloatFromIntPtr (IntPtr data, int offset) {
		byte[] byte_array = new byte[4];
		for (int j = 0; j < 4; ++j) {
			byte_array [j] = Marshal.ReadByte (data, offset + j);
		}
		return BitConverter.ToSingle (byte_array, 0);
	}

	public int LoadMeshFromAsset(string asset_name, ref Mesh mesh) {
		TextAsset asset = Resources.Load(asset_name, typeof(TextAsset)) as TextAsset;
		if (asset == null) {
			Debug.Log ("Didn't load file!");
			return -1;
		}
		byte[] bunny_data = asset.bytes;
		Debug.Log (bunny_data.Length.ToString ());
		if (bunny_data.Length == 0) {
			Debug.Log ("Didn't load bunny!");
			return -1;
		}
		return DecodeMesh (bunny_data, ref mesh);
	}
	/*
	public IEnumerator LoadMeshFromURL(string url, ref Mesh mesh) {
		WWW www = new WWW (url);
		yield return www;
		if (www.bytes.Length == 0)
			return -1;
		return DecodeMesh (www.bytes, ref mesh);
	}
	*/

	public unsafe int DecodeMesh (byte[] data, ref Mesh mesh) {
		DracoToUnityMesh* tmp_mesh;
		if (DecodeMeshForUnity (data, data.Length, &tmp_mesh) <= 0) {
			Debug.Log ("Failed: Decoding error.");
			return -1;
		}

		Debug.Log ("Num indices: " + tmp_mesh->num_faces.ToString ());
		Debug.Log ("Num vertices: " + tmp_mesh->num_vertices.ToString ());

		int num_faces = tmp_mesh->num_faces;
		int[] new_triangles = new int[tmp_mesh->num_faces * 3];
		for (int i = 0; i < tmp_mesh->num_faces; ++i) {
			new_triangles[i * 3] = Marshal.ReadInt32 (tmp_mesh->indices, i * 3 * 4);
			new_triangles[i * 3 + 1] = Marshal.ReadInt32 (tmp_mesh->indices, i * 3 * 4 + 4);
			new_triangles[i * 3 + 2] = Marshal.ReadInt32 (tmp_mesh->indices, i * 3 * 4 + 8);
		}

		// For floating point numbers, there's no Marshal functions could directly read from the unmanaged data.
		// TODO: Find better way to read float numbers.
		Vector3[] new_vertices = new Vector3[tmp_mesh->num_vertices];
		int byte_stride_per_value = 4;
		int num_value_per_vertex = 3;
		int byte_stride_per_vertex = byte_stride_per_value * num_value_per_vertex;
		for (int i = 0; i < tmp_mesh->num_vertices; ++i) {
			for (int j = 0; j < 3; ++j) {
				new_vertices [i] [j] = 
					ReadFloatFromIntPtr (tmp_mesh->position, i * byte_stride_per_vertex + byte_stride_per_value * j) * 60;
			}
		}

		Marshal.FreeCoTaskMem(tmp_mesh->position);
		Marshal.FreeCoTaskMem((IntPtr)tmp_mesh);

		mesh.vertices = new_vertices;     
		mesh.triangles = new_triangles;

		mesh.RecalculateBounds();
		mesh.RecalculateNormals();

		return num_faces;
	}
}
