using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

public class DracoDecodingObject : MonoBehaviour {

	public TextMesh debug_text;

	// Use this for initialization
	void Start () {
		debug_text = GetComponentInChildren<TextMesh> ();

		List<Mesh> mesh = new List<Mesh>();
		DracoMeshLoader draco_loader = new DracoMeshLoader ();
		int num_faces = draco_loader.LoadMeshFromAsset ("bunny", ref mesh);

		// TODO: Meshes.
		if (num_faces > 0) {
			GetComponent<MeshFilter> ().mesh = mesh[0];
		}

		debug_text.text = num_faces.ToString ();
	}
	
	// Update is called once per frame
	void Update () {
		transform.Rotate(0, 10 * Time.deltaTime, 0);
	}
}
