using System;
using System.Collections.Generic;
using OscJack;
using Unity.VisualScripting;
using UnityEngine;

public class StateTransition : MonoBehaviour
{
    public enum State
    {
        Play = 0,
        Pause = 1,
        Standby = 2,
        Finish = 3
    }

    [SerializeField] private List<String> _ipAddresses;
    private List<OscClient> _clients = new();
    private static readonly int sendPort = 22222;

    public State CurrentState { get; private set; } = State.Standby;

    private readonly Dictionary<State, GameObject> _gameViewPanels = new();
    [SerializeField] private GameObject _pausePanel;
    [SerializeField] private GameObject _playPanel;
    [SerializeField] private GameObject _finishPanel;
    [SerializeField] private GameObject _standbyPanel;

    [SerializeField] private List<TwistViewer> _twistViewers;
    [SerializeField] private List<TwistGetter> _twistGetters;

    private void Start()
    {
        // ゲーム状態によって描画を操作するパネルを辞書に格納する
        StorePanelInDict();

        PlayToFinish();
        FinishToStandby();

        InitOscClient();
    }

    private void InitOscClient()
    {
        foreach (var address in _ipAddresses)
        {
            _clients.Add(new OscClient(address, sendPort));
        }
    }


    private void Update()
    {
        // Spaceキー押下を検知したらフラグを更新する
        if (Input.GetKeyDown(KeyCode.Space))
        {
            switch (CurrentState)
            {
                // プレイ状態から一時停止状態に移行する
                case State.Play:
                    PlayToPause();
                    break;

                // 一時停止状態からプレイ状態に移行する
                case State.Pause:
                    PauseToPlay();
                    break;
            }
        }

        // Returnキー押下を検知したらフラグを更新する
        if (Input.GetKeyDown(KeyCode.Return))
        {
            switch (CurrentState)
            {
                // 待機状態からプレイ状態に移行する 
                case State.Standby:
                    StandbyToPlay();
                    break;

                // プレイ状態から終了状態に移行する
                case State.Play:
                    PlayToFinish();
                    break;

                // 終了状態から待機状態に移行する 
                case State.Finish:
                    FinishToStandby();
                    break;
            }
        }
    }

    private void StorePanelInDict()
    {
        _gameViewPanels.Add(State.Play, _playPanel.gameObject);
        _gameViewPanels.Add(State.Pause, _pausePanel);
        _gameViewPanels.Add(State.Standby, _standbyPanel);
        _gameViewPanels.Add(State.Finish, _finishPanel);

        _gameViewPanels[State.Play].SetActive(false);
        _gameViewPanels[State.Pause].SetActive(false);
        _gameViewPanels[State.Standby].SetActive(true);
        _gameViewPanels[State.Finish].SetActive(false);
    }


    public void PlayToPause()
    {
        if (CurrentState != State.Play) return;

        _gameViewPanels[State.Play].SetActive(false);
        _gameViewPanels[State.Pause].SetActive(true);

        foreach (var twistGetter in _twistGetters)
            twistGetter.SetActiveOsc(false);

        Debug.Log("Play to Pause");
        CurrentState = State.Pause;
    }

    public void PauseToPlay()
    {
        if (CurrentState != State.Pause) return;

        _gameViewPanels[State.Pause].SetActive(false);
        _gameViewPanels[State.Play].SetActive(true);

        foreach (var twistGetter in _twistGetters)
            twistGetter.SetActiveOsc(true);

        Debug.Log("Pause to Play");
        CurrentState = State.Play;
    }


    public void StandbyToPlay()
    {
        if (CurrentState != State.Standby) return;

        _gameViewPanels[State.Standby].SetActive(false);
        _gameViewPanels[State.Play].SetActive(true);

        foreach (var twistGetter in _twistGetters)
            twistGetter.SetActiveOsc(true);

        Debug.Log("Standby to Play");
        CurrentState = State.Play;
    }


    public void PlayToFinish()
    {
        if (CurrentState != State.Play && CurrentState != State.Finish) return;

        _gameViewPanels[State.Play].SetActive(false);
        _gameViewPanels[State.Finish].SetActive(true);

        foreach (var twistGetter in _twistGetters)
            twistGetter.SetActiveOsc(false);

        foreach (var twistViewer in _twistViewers)
            twistViewer._isVisibleTwistCount = false;

        Debug.Log("Play to Finish");
        CurrentState = State.Finish;
    }


    public void FinishToStandby()
    {
        if (CurrentState != State.Finish) return;

        _gameViewPanels[State.Finish].SetActive(false);
        _gameViewPanels[State.Standby].SetActive(true);

        foreach (var twistGetter in _twistGetters)
        {
            twistGetter.ResetRotation();
            twistGetter.ResetTwistContainers();
        }
        
        foreach (var twistViewer in _twistViewers)
            twistViewer._isVisibleTwistCount = true;

        Debug.Log("Finish to Standby");
        CurrentState = State.Standby;
    }

    public void ShowResultScore()
    {
        if (CurrentState != State.Finish) return;
        _gameViewPanels[State.Finish].SetActive(false);
    }


    public void ResetTwistCount()
    {
        foreach (var twistGetter in _twistGetters)
        {
            twistGetter.ResetTwistContainers();
        }
    }

    public void ToggleVisibilityTwistCount()
    {
        foreach (var twistViewer in _twistViewers)
        {
            twistViewer._isVisibleTwistCount = !twistViewer._isVisibleTwistCount;
        }
    }

    public void SendResetIMUMessage()
    {
        foreach (var client in _clients)
            client.Send("/reset/imu");
    }
}