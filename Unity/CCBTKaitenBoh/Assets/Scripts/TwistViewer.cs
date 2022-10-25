using System;
using System.Collections;
using System.Collections.Generic;
using System.Globalization;
using Michsky.UI.ModernUIPack;
using TMPro;
using Unity.VisualScripting;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Serialization;

public class TwistViewer : MonoBehaviour
{
    [SerializeField] private GameObject _twistTarget;
    [SerializeField] private Transform _xyzPanel;

    // todo: 別スクリプトに移動する
    [SerializeField] private AudioSource _audioSource;

    // 音源
    [SerializeField] private List<AudioClip> _audioClips;

    private Dictionary<string, TwistGetter.TwistContainer> twistContainers = new();
    private readonly Dictionary<string, TextMeshProUGUI> twistCountTexts = new();
    private readonly Dictionary<string, ProgressBar> twistProgressBars = new();

    private Dictionary<string, double> previousCounts = new()
    {
        { "X", 0 },
        { "Y", 0 },
        { "Z", 0 },
    };

    private Dictionary<string, Image> progressBarImages = new(){
        {"X", null},
        {"Y", null},
        {"Z", null}
    };

    public bool _isVisibleTwistCount = true;

    private void Start()
    {
        twistContainers = _twistTarget.GetComponent<TwistGetter>().twistContainers;

        foreach (var ax in new[] { "X", "Y", "Z" })
        {
            twistCountTexts[ax] = _xyzPanel.Find($"{ax}Panel/TwistCount").GetComponent<TextMeshProUGUI>();
            twistProgressBars[ax] = _xyzPanel.Find($"{ax}Panel/AnglePB").GetComponent<ProgressBar>();
            progressBarImages[ax] = twistProgressBars[ax].loadingBar;
        }

        // 角度を表示するバーのプリフィックス表示を有効化する
        foreach (var bar in twistProgressBars.Values)
            bar.addPrefix = true;
    }


    void Update()
    {
        foreach (var ax in new[] { "X", "Y", "Z" })
        {
            var container = twistContainers[ax];
            var count = container.GetTwistCount();
            twistCountTexts[ax].text = _isVisibleTwistCount
                ? Math.Abs(count).ToString(CultureInfo.CurrentCulture)
                // ? count.ToString(CultureInfo.CurrentCulture) // マイナス表示あり
                : "?";

            // 前回のカウントより増えていたらSEを再生する
            if (Math.Abs(count) > previousCounts[ax])
                _audioSource.PlayOneShot(_audioClips[0]);
            else if (Math.Abs(count) < previousCounts[ax])
                _audioSource.PlayOneShot(_audioClips[1]);

            previousCounts[ax] = Math.Abs(count);

            var degree = container.GetTotalTwistDegree(true) % 360;
            if (degree >= 0)
            {
                twistProgressBars[ax].prefix = "";
                twistProgressBars[ax].ChangeValue(degree);
                progressBarImages[ax].color = Color.white;
            }
            else
            {
                twistProgressBars[ax].prefix = "−";
                twistProgressBars[ax].ChangeValue(360 - Math.Abs(degree));
                progressBarImages[ax].color = Color.white;
            }
        }
    }
}