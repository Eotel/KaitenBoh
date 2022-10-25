using System;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEditor.Build.Content;
using UnityEngine;

public class TwistGetter : MonoBehaviour
{
    // public readonly List<TwistContainer> TwistContainers = new();
    public readonly Dictionary<string, TwistContainer> twistContainers = new();


    /// <summary>
    ///     ある軸周りの回転角度及び回転数を格納するクラス
    /// </summary>
    public class TwistContainer
    {
        public Vector3 Axis { get; }
        private float TotalTwistDegree { get; set; }
        public Quaternion previousRotation;
        public float offset;
        public Quaternion offsetRotation;

        public TwistContainer(Vector3 axis)
        {
            Axis = axis;
            ResetPreviousRotation();
            ResetTotalTwistDegree();
            offset = 0.0f;
        }

        public void UpdateTwistDegree(Quaternion rotation)
        {
            var twist = Twist.GetTwistAroundAxis(previousRotation, rotation, Axis);
            if (twist > 180) twist -= 360.0f; // 正負の回転方向がある

            TotalTwistDegree += twist;
            previousRotation = rotation;
        }

        public double GetTwistCount()
        {
            return Math.Truncate((TotalTwistDegree - offset) / 360.0f);
        }

        public float GetTotalTwistDegree(Boolean withOffset = false)
        {
            return withOffset ? TotalTwistDegree - offset : TotalTwistDegree;
        }

        public void ResetPreviousRotation()
        {
            previousRotation = Quaternion.identity;
        }

        public void ResetTotalTwistDegree()
        {
            TotalTwistDegree = 0.0f;
        }
    }

    private void Awake()
    {
        twistContainers["X"] = new TwistContainer(Vector3.right);
        twistContainers["Y"] = new TwistContainer(Vector3.up);
        twistContainers["Z"] = new TwistContainer(Vector3.forward);
    }

    private void Update()
    {
        if (twistContainers is null) return;

        var rotation = transform.localRotation;
        foreach (var container in twistContainers.Values)
            container.UpdateTwistDegree(rotation);
    }

    public void SetActiveOsc(bool value)
    {
        var machine = gameObject.GetComponent<ScriptMachine>();
        machine.enabled = value;
    }

    public void ResetTwistContainers()
    {
        foreach (var (ax, twistContainer) in twistContainers)
        {
            twistContainer.ResetTotalTwistDegree();
        }
    }

    public void ResetRotation()
    {
        transform.localRotation = Quaternion.identity;
    }
}