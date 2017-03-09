using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using AudioGame;

public class Manager : MonoBehaviour {

    // reads the .sm file
    AudioParser parser;
    // tracks the "playhead"
    float songTime = 0;
    // the last index retrieved
    int lastIndex = 0;

    // THESE THINGS SHOULD BE READ IN FROM THE FILE
    // BUT THEY'RE HARDCODED FOR NOW
    // time offset (delay when starting the song)
    float timeOffset = -10;
    // the first index of the actual beats
    int indexOffset = 38;
    // beats per minute???
    float bpms = 210;

    // convenience for later
    float beatsPerSec;

    // distance between beats in unity space
    float noteDist = 10;

    // number of beats that are on screen at a time
    int previewBeats = 10;

    public Note NotePrefab;
    public ParticleSystem ParticlePrefab;

    // note gameobjects
    ArrayList notes;

    // player ref
    PlayerMovement player;

    // psuedo singleton
    public static Manager instance;

    // offscreen location to hide gameobjects
    Vector3 offscreenPos = new Vector3(0, -10000, -10000);

    // Use this for initialization
    void Start() {
        // psuedo singleton
        if (Manager.instance != null)
        {
            Destroy(this);
        } else
        {
            Manager.instance = this;
        }

        // beats per second
        beatsPerSec = bpms / 60;

        // index offset
        indexOffset -= previewBeats;

        // start the parser
        parser = new AudioParser();

        // text mesh
        TextMesh tm = FindObjectOfType<TextMesh>();
        if (tm != null)
        {
            tm.text = parser.LoadMessage();
        }

        // notes arraylist
        notes = new ArrayList();

        // get player
        player = FindObjectOfType<PlayerMovement>();
	}
	
	// Update is called once per frame
	void Update () {
        // increase time
        songTime += Time.deltaTime * beatsPerSec;

        int t = Mathf.FloorToInt(songTime - timeOffset);
        if (t > lastIndex)
        {
            // get the next beat
            string noteMap = parser.GetItem(t + indexOffset + previewBeats);
            print(noteMap);
            CreateNote(noteMap);
            lastIndex = t;
        }

        MoveNotes();
	}

    void MoveNotes()
    {
        foreach (Note note in notes)
        {
            Vector3 cur = note.transform.position;
            // temporal parameter 0->1
            float a = (songTime - note.startTime);
            // destroy if unneeded;
            if (a > previewBeats * 2)
            {
                notes.Remove(note);
                Destroy(note.gameObject);
            }
            // z position
            float z = noteDist * previewBeats - noteDist * a;
            if (-1 < z && z < 1)
            {
                if (player.transform.position.x == note.transform.position.x)
                {
                    notes.Remove(note);
                    Destroy(note.gameObject);
                    GameObject.Destroy(GameObject.Instantiate(ParticlePrefab, player.transform.position, Quaternion.identity, null).gameObject,5);
                }
            }
            note.transform.position = new Vector3(cur.x, cur.y, z);
        }
    }

    void CreateNote(string noteMap)
    {
        if (noteMap.Length < 4) return;

        int i = 0;
        for (i = 0; i < 4; i++)
        {
            int x = i % 4;
            if (noteMap[i] != '0')
            {
                Note note = GameObject.Instantiate(NotePrefab, new Vector3(-2.25f + 1.5f * x, 0, 100), Quaternion.identity, null);
                note.startTime = songTime;
                notes.Add(note);
                break;
            }
        }
    }
}
