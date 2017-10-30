using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

public unsafe class DracoMeshLoader
{
	// Must stay the order to be consistent with C++ interface.
	[ StructLayout (LayoutKind.Sequential)]
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

	private struct DecodedMesh
	{
		public int[] faces;
		public Vector3[] vertices;
	}

	[DllImport ("dracodec_unity")]
	private static extern int DecodeMeshForUnity (byte[] buffer, int length, DracoToUnityMesh**tmp_mesh);

	static private int max_num_vertices_per_mesh = 60000;

	// Unity only support maximum 65534 vertices per mesh. So large meshes need to be splited.
	private void SplitMesh (DecodedMesh mesh, ref List<DecodedMesh> splited_meshes)
	{
		List<int> faces_left = new List<int> ();
		for (int i = 0; i < mesh.faces.Length; ++i) {
			faces_left.Add (mesh.faces [i]);
		}
		int num_sub_meshes = 0;

		while (faces_left.Count > 0) {
			Debug.Log ("Faces left: " + faces_left.Count.ToString ());
			num_sub_meshes++;
			List<int> tmp_left_faces = new List<int> ();
			List<int> faces_extracted = new List<int> ();
			List<Vector3> vertices_extracted = new List<Vector3> ();

			int unique_corner_id = 0;
			Dictionary<int, int> index_to_new_index = new Dictionary<int ,int> ();
			for (int face = 0; face < faces_left.Count / 3; ++face) {
				bool pick_face = true;
				// If all indices has appeared or there's still space for more vertices.
				for (int corner = 0; corner < 3; ++corner) {
					if (!index_to_new_index.ContainsKey (faces_left [face * 3 + corner])) {
						if (unique_corner_id < max_num_vertices_per_mesh) {
							index_to_new_index.Add (faces_left [face * 3 + corner], unique_corner_id);
							vertices_extracted.Add (mesh.vertices [faces_left [face * 3 + corner]]);
							unique_corner_id++;
						} else {
							// Can not take more new vertices.
							pick_face = false;
						}
					}
				}
				// Add face to this sub mesh.
				if (pick_face) {
					for (int corner = 0; corner < 3; ++corner) {
						faces_extracted.Add (index_to_new_index [faces_left [face * 3 + corner]]);
					}
				} else {
					// Otherwise save face for the next sub-mesh.
					for (int corner = 0; corner < 3; ++corner) {
						tmp_left_faces.Add (faces_left [face * 3 + corner]);
					}
				}
			}

			DecodedMesh sub_mesh = new DecodedMesh ();
			sub_mesh.faces = faces_extracted.ToArray ();
			sub_mesh.vertices = vertices_extracted.ToArray ();
			splited_meshes.Add (sub_mesh);

			faces_left = tmp_left_faces;
		}
	}

	private float ReadFloatFromIntPtr (IntPtr data, int offset)
	{
		byte[] byte_array = new byte[4];
		for (int j = 0; j < 4; ++j) {
			byte_array [j] = Marshal.ReadByte (data, offset + j);
		}
		return BitConverter.ToSingle (byte_array, 0);
	}

	public int LoadMeshFromAsset (string asset_name, ref List<Mesh> meshes)
	{
		TextAsset asset = Resources.Load (asset_name, typeof(TextAsset)) as TextAsset;
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
		return DecodeMesh (bunny_data, ref meshes);
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

	public unsafe int DecodeMesh (byte[] data, ref List<Mesh> meshes)
	{
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
			new_triangles [i * 3] = Marshal.ReadInt32 (tmp_mesh->indices, i * 3 * 4);
			new_triangles [i * 3 + 1] = Marshal.ReadInt32 (tmp_mesh->indices, i * 3 * 4 + 4);
			new_triangles [i * 3 + 2] = Marshal.ReadInt32 (tmp_mesh->indices, i * 3 * 4 + 8);
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

		Marshal.FreeCoTaskMem (tmp_mesh->position);
		Marshal.FreeCoTaskMem ((IntPtr)tmp_mesh);

		if (new_vertices.Length > 61000) {
			DecodedMesh decoded_mesh = new DecodedMesh ();
			decoded_mesh.vertices = new_vertices;
			decoded_mesh.faces = new_triangles;
			List<DecodedMesh> splited_meshes = new List<DecodedMesh> ();

			SplitMesh (decoded_mesh, ref splited_meshes);
			for (int i = 0; i < splited_meshes.Count; ++i) {
				Mesh mesh = new Mesh ();
				mesh.vertices = splited_meshes [i].vertices;
				mesh.triangles = splited_meshes [i].faces;

				mesh.RecalculateBounds ();
				mesh.RecalculateNormals ();
				meshes.Add (mesh);
			}
		} else {
			// Mesh not that big.
			Mesh mesh = new Mesh ();
			mesh.vertices = new_vertices;
			mesh.triangles = new_triangles;

			// TODO: Duplicated code.
			mesh.RecalculateBounds ();
			mesh.RecalculateNormals ();
			meshes.Add (mesh);
		}
			
		return num_faces;
	}
}
