{
  pcbis.pas — PCBoard Internet Services daemon
  openwatcomirc project

  Main entry point. Initializes config, starts listeners, runs event loop.

  Usage:
    pcbis                   — run in foreground (console logging)
    pcbis -d                — run as daemon (background, log to file)
    pcbis -c pcbis.cfg      — specify config file
    pcbis -t                — test config and exit
}

program pcbis;

{$mode objfpc}{$H+}

uses
  SysUtils, Classes, BaseUnix, Unix, Sockets,
  pcbis_config, pcbis_log, pcbis_net, pcbis_telnet, pcbis_binkp,
  pcbis_ftp, pcbis_http, pcbis_smtp, pcbis_events, pcbis_nodedata, pcbfoss;

const
  PCBIS_VERSION = '0.1.0';
  PCBIS_BANNER  = 'pcbis ' + PCBIS_VERSION + ' — PCBoard Internet Services';
  DEFAULT_CFG   = 'pcbis.cfg';

var
  ConfigFile : string;
  Daemonize  : boolean;
  TestOnly   : boolean;
  Running    : boolean;
  Cfg        : TPcbisConfig;
  Server     : TPcbisServer;

procedure ShowUsage;
begin
  WriteLn(PCBIS_BANNER);
  WriteLn;
  WriteLn('Usage: pcbis [options]');
  WriteLn('  -c <file>   Config file (default: pcbis.cfg)');
  WriteLn('  -d          Daemonize (run in background)');
  WriteLn('  -t          Test config and exit');
  WriteLn('  -v          Show version');
  WriteLn('  -h          Show this help');
  Halt(0);
end;

procedure ShowVersion;
begin
  WriteLn(PCBIS_BANNER);
  WriteLn('Built with fpc264irc');
  Halt(0);
end;

procedure ParseArgs;
var
  I : integer;
begin
  ConfigFile := DEFAULT_CFG;
  Daemonize := False;
  TestOnly := False;

  I := 1;
  while I <= ParamCount do
  begin
    if ParamStr(I) = '-c' then
    begin
      Inc(I);
      if I > ParamCount then
      begin
        WriteLn('Error: -c requires a filename');
        Halt(1);
      end;
      ConfigFile := ParamStr(I);
    end
    else if ParamStr(I) = '-d' then
      Daemonize := True
    else if ParamStr(I) = '-t' then
      TestOnly := True
    else if ParamStr(I) = '-v' then
      ShowVersion
    else if (ParamStr(I) = '-h') or (ParamStr(I) = '--help') then
      ShowUsage
    else
    begin
      WriteLn('Unknown option: ', ParamStr(I));
      Halt(1);
    end;
    Inc(I);
  end;
end;

procedure SignalHandler(Sig : cint); cdecl;
begin
  case Sig of
    SIGHUP:
      begin
        LogInfo('SIGHUP received — reloading config');
        Cfg.Reload(ConfigFile);
      end;
    SIGTERM, SIGINT:
      begin
        LogInfo('Shutdown signal received');
        Running := False;
      end;
  end;
end;

procedure InstallSignals;
begin
  fpSignal(SIGINT, @SignalHandler);
  fpSignal(SIGTERM, @SignalHandler);
  fpSignal(SIGHUP, @SignalHandler);
  fpSignal(SIGPIPE, SIG_IGN); { ignore broken pipe } { ignore broken pipe }
end;

procedure WritePidFile(const Filename : string);
var
  F : TextFile;
begin
  try
    AssignFile(F, Filename);
    Rewrite(F);
    WriteLn(F, fpGetPid);
    CloseFile(F);
  except
    on E: Exception do
      LogError('Failed to write PID file: ' + E.Message);
  end;
end;

procedure DoDaemonize;
var
  Pid : TPid;
begin
  { First fork - parent exits }
  Pid := fpFork;
  if Pid < 0 then
  begin
    WriteLn('Fork failed');
    Halt(1);
  end;
  if Pid > 0 then
    Halt(0);

  { Child becomes session leader }
  fpSetSid;

  { Second fork - prevent acquiring controlling terminal }
  Pid := fpFork;
  if Pid < 0 then
    Halt(1);
  if Pid > 0 then
    Halt(0);

  { Redirect stdio to /dev/null }
  AssignFile(Input, '/dev/null');
  Reset(Input);
  AssignFile(Output, '/dev/null');
  Rewrite(Output);
end;

{ === Main === }
begin
  ParseArgs;

  WriteLn(PCBIS_BANNER);
  WriteLn;

  { Load config }
  Cfg := TPcbisConfig.Create;
  try
    if not Cfg.LoadFromFile(ConfigFile) then
    begin
      WriteLn('Error: failed to load config from ', ConfigFile);
      Halt(1);
    end;

    LogInfo('Config loaded from ' + ConfigFile);

    if TestOnly then
    begin
      WriteLn('Config OK');
      Cfg.Dump;
      Halt(0);
    end;

    { Init logging }
    if Daemonize then
      LogInit(Cfg.LogFile)
    else
      LogInit(''); { stdout }

    LogInfo(PCBIS_BANNER + ' starting');

    { Daemonize if requested }
    if Daemonize then
    begin
      DoDaemonize;
      WritePidFile(Cfg.PidFile);
    end;

    InstallSignals;

    { Create server and start listeners }
    Server := TPcbisServer.Create(Cfg);
    try
      if Cfg.TelnetEnabled then
      begin
        Server.StartTelnet;
        LogInfo('Telnet listener on ' + Cfg.TelnetListen + ':' + IntToStr(Cfg.TelnetPort));
      end;

      if Cfg.BinkpEnabled then
      begin
        Server.StartBinkp;
        LogInfo('BinkP listener on ' + Cfg.BinkpListen + ':' + IntToStr(Cfg.BinkpPort));
      end;

      Server.StartFtp;
      LogInfo('FTP listener active');

      Server.StartHttp;
      LogInfo('HTTP listener active');

      if Cfg.SmtpEnabled then
      begin
        Server.StartSmtp;
        LogInfo('SMTP outbound queue active, relay: ' + Cfg.SmtpRelay);
      end;

      LogInfo('pcbis ready — ' + IntToStr(Cfg.MaxConnections) + ' max connections');

      { Main event loop }
      Running := True;
      while Running do
      begin
        Server.Poll(1000); { 1 second timeout }
      end;

      LogInfo('Shutting down...');
      Server.StopAll;
    finally
      Server.Free;
    end;

    { Cleanup }
    if Daemonize and FileExists(Cfg.PidFile) then
      DeleteFile(Cfg.PidFile);

    LogInfo('pcbis stopped');
    LogFini;
  finally
    Cfg.Free;
  end;
end.
