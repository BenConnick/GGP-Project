using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerMovement : MonoBehaviour {

    public int rail = 1;

    public float railX0 = -1; //the X coordinate of the first rail
    public float railXdelta = 1; //the change in x between rails
    private float[] railXcoords; // the x coordinates for each rail

    private int railCount = 4; //total number of rails

	// Use this for initialization
	void Start () {
        //load coordinates for each rail
        railXcoords = new float[4];
        for (int i = 0; i < railCount; i++) {
            railXcoords[i] = railX0 + railXdelta * i;
        }
	}
	
	// Update is called once per frame
	void Update () {
        //position player based on current rail
        Vector3 newPos = transform.position;
        newPos.x = railXcoords[rail];
        transform.position = newPos;

        //cycle through rails with left and right arrows
        if (Input.GetKeyDown(KeyCode.LeftArrow))
        {
            if (rail > 0)
            {
                rail -= 1;
            }
        }
        if (Input.GetKeyDown(KeyCode.RightArrow))
        {
            if (rail < railCount-1)
            {
                rail += 1;
            }
        }
    }
}
