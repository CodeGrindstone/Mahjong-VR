using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MoveWithXR : MonoBehaviour
{
    public Transform XRCamera_Pos;
    public Transform XRLeftControl_Pos;
    public Transform XRRightControl_Pos;

    public Transform playerRootPos;
    public Transform playerLeftHandPos;
    public Transform playerRightHandPos;
    public Transform playerHeadPos;

    void Update()
    {
        playerRootPos.transform.SetPositionAndRotation(new Vector3(XRCamera_Pos.position.x,
                                                                                                               XRCamera_Pos.position.y - 1,
                                                                                                               XRCamera_Pos.position.z + 0.5f), 
                                                                                                               playerRootPos.rotation); 
       playerRightHandPos.rotation = XRRightControl_Pos.rotation;
       playerLeftHandPos.rotation = XRLeftControl_Pos.rotation;
        
        Vector3 direction = XRCamera_Pos.position - transform.position;
        direction.y = 0;        
        if (direction != Vector3.zero) 
        {
            Quaternion rotation = Quaternion.LookRotation(direction);
           playerHeadPos.rotation = rotation;
        }
    }

}
