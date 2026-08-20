{
  pcbis_binkp.pas — BinkP protocol handler for pcbis
  Implements FSP-1018 BinkP/1.1 for FidoNet mail exchange.
  Incoming packets are dropped to QFront's inbound directory.
}

unit pcbis_binkp;

{$mode objfpc}{$H+}

interface

uses
  pcbis_net;

procedure BinkpOnConnect(Conn : TPcbisConnection);
procedure BinkpOnData(Conn : TPcbisConnection);
procedure BinkpOnDisconnect(Conn : TPcbisConnection);

implementation

uses
  SysUtils, Classes, pcbis_log;

const
  { BinkP frame types (command frames have bit 15 set) }
  BINKP_DATA_FRAME = 0;
  BINKP_CMD_FRAME  = $8000;

  { BinkP commands }
  M_NUL  = 0;   { site info }
  M_ADR  = 1;   { address list }
  M_PWD  = 2;   { password }
  M_FILE = 3;   { file header }
  M_OK   = 4;   { password accepted }
  M_EOB  = 5;   { end of batch }
  M_GOT  = 6;   { file received OK }
  M_ERR  = 7;   { error }
  M_BSY  = 8;   { busy }
  M_GET  = 9;   { get file }
  M_SKIP = 10;  { skip file }

type
  TBinkpState = (
    bsWaitAddr,     { waiting for M_ADR }
    bsWaitPwd,      { waiting for M_PWD }
    bsSession,      { authenticated, transferring }
    bsReceiving,    { receiving a file }
    bsDone          { EOB received }
  );

  TBinkpSession = record
    State       : TBinkpState;
    RemoteAddr  : string;   { FidoNet address }
    RemoteSysop : string;
    RemoteSys   : string;
    Authenticated : boolean;
    CurrentFile : string;
    CurrentSize : longint;
    BytesRecv   : longint;
    FileStream  : TFileStream;
  end;

var
  Sessions : array[0..MAX_CLIENTS-1] of TBinkpSession;

function GetSession(Conn : TPcbisConnection) : integer;
var
  I : integer;
begin
  { Simple — use socket as index mod array size }
  Result := Conn.Socket mod MAX_CLIENTS;
end;

procedure SendCmd(Conn : TPcbisConnection; Cmd : byte; const Data : string);
var
  Len   : word;
  Frame : string;
begin
  Len := Length(Data) + 1;  { +1 for command byte }
  SetLength(Frame, 2 + Len);
  Frame[1] := chr((Len or BINKP_CMD_FRAME) shr 8);
  Frame[2] := chr(Len and $FF);
  Frame[3] := chr(Cmd);
  if Length(Data) > 0 then
    Move(Data[1], Frame[4], Length(Data));
  Conn.OutBuf := Conn.OutBuf + Frame;
end;

procedure SendNUL(Conn : TPcbisConnection; const Info : string);
begin
  SendCmd(Conn, M_NUL, Info);
end;

procedure BinkpOnConnect(Conn : TPcbisConnection);
var
  Idx : integer;
begin
  Idx := GetSession(Conn);
  FillChar(Sessions[Idx], SizeOf(TBinkpSession), 0);
  Sessions[Idx].State := bsWaitAddr;

  { Send our info }
  SendNUL(Conn, 'SYS PCBoard BBS');
  SendNUL(Conn, 'ZYZ Sysop');
  SendNUL(Conn, 'VER pcbis/0.1.0 binkp/1.1');
  SendNUL(Conn, 'TIME ' + FormatDateTime('ddd, dd mmm yyyy hh:nn:ss', Now));

  { Send our address — from config }
  { TODO: get from config }
  SendCmd(Conn, M_ADR, '1:1/0@fidonet');

  LogConn('BINKP', Conn.RemoteIP, 'session initiated');
end;

procedure ProcessFrame(Conn : TPcbisConnection; IsCmd : boolean; Cmd : byte; const Data : string);
var
  Idx : integer;
begin
  Idx := GetSession(Conn);

  if not IsCmd then
  begin
    { Data frame — file content }
    if Sessions[Idx].State = bsReceiving then
    begin
      if Assigned(Sessions[Idx].FileStream) then
      begin
        Sessions[Idx].FileStream.Write(Data[1], Length(Data));
        Sessions[Idx].BytesRecv := Sessions[Idx].BytesRecv + Length(Data);

        if Sessions[Idx].BytesRecv >= Sessions[Idx].CurrentSize then
        begin
          Sessions[Idx].FileStream.Free;
          Sessions[Idx].FileStream := nil;
          Sessions[Idx].State := bsSession;
          LogConn('BINKP', Conn.RemoteIP,
                  'received ' + Sessions[Idx].CurrentFile +
                  ' (' + IntToStr(Sessions[Idx].BytesRecv) + ' bytes)');
          SendCmd(Conn, M_GOT, Sessions[Idx].CurrentFile + ' ' +
                  IntToStr(Sessions[Idx].CurrentSize) + ' 0');
        end;
      end;
    end;
    Exit;
  end;

  { Command frame }
  case Cmd of
    M_NUL:
      begin
        { Info string — parse SYS, ZYZ, VER, etc. }
        if Pos('SYS ', Data) = 1 then
          Sessions[Idx].RemoteSys := Copy(Data, 5, Length(Data))
        else if Pos('ZYZ ', Data) = 1 then
          Sessions[Idx].RemoteSysop := Copy(Data, 5, Length(Data));
        LogDebug('BINKP NUL: ' + Data);
      end;

    M_ADR:
      begin
        Sessions[Idx].RemoteAddr := Data;
        Sessions[Idx].State := bsWaitPwd;
        LogConn('BINKP', Conn.RemoteIP, 'address: ' + Data);
      end;

    M_PWD:
      begin
        { TODO: verify password against config }
        Sessions[Idx].Authenticated := True;
        Sessions[Idx].State := bsSession;
        SendCmd(Conn, M_OK, 'secure');
        LogConn('BINKP', Conn.RemoteIP, 'authenticated');
      end;

    M_FILE:
      begin
        { File header: "filename size time offset" }
        Sessions[Idx].CurrentFile := Data; { simplified — parse properly }
        Sessions[Idx].CurrentSize := 0;
        Sessions[Idx].BytesRecv := 0;
        Sessions[Idx].State := bsReceiving;

        { TODO: parse size from header, open file in QFront inbound dir }
        { For now, log it }
        LogConn('BINKP', Conn.RemoteIP, 'receiving file: ' + Data);
      end;

    M_EOB:
      begin
        Sessions[Idx].State := bsDone;
        SendCmd(Conn, M_EOB, '');
        LogConn('BINKP', Conn.RemoteIP, 'end of batch');
        Conn.State := csClosing;
      end;

    M_ERR:
      begin
        LogConn('BINKP', Conn.RemoteIP, 'error: ' + Data);
        Conn.State := csClosing;
      end;

    M_BSY:
      begin
        LogConn('BINKP', Conn.RemoteIP, 'remote busy: ' + Data);
        Conn.State := csClosing;
      end;
  end;
end;

procedure BinkpOnData(Conn : TPcbisConnection);
var
  FrameLen : word;
  IsCmd    : boolean;
  Cmd      : byte;
  Data     : string;
begin
  { BinkP framing: 2-byte header (bit 15 = cmd flag, bits 14-0 = length) }
  while Length(Conn.InBuf) >= 2 do
  begin
    FrameLen := (ord(Conn.InBuf[1]) shl 8) or ord(Conn.InBuf[2]);
    IsCmd := (FrameLen and BINKP_CMD_FRAME) <> 0;
    FrameLen := FrameLen and $7FFF;
    { BUG-3 fix: enforce max frame size (8KB) }
    if FrameLen > 8192 then begin
      LogError('BinkP: frame too large: ' + IntToStr(FrameLen));
      DisconnectSession(Conn);
      Exit;
    end;

    if Length(Conn.InBuf) < 2 + FrameLen then
      Break; { incomplete frame — wait for more data }

    if IsCmd and (FrameLen > 0) then
    begin
      Cmd := ord(Conn.InBuf[3]);
      Data := Copy(Conn.InBuf, 4, FrameLen - 1);
      ProcessFrame(Conn, True, Cmd, Data);
    end
    else if not IsCmd then
    begin
      Data := Copy(Conn.InBuf, 3, FrameLen);
      ProcessFrame(Conn, False, 0, Data);
    end;

    Delete(Conn.InBuf, 1, 2 + FrameLen);
  end;
end;

procedure BinkpOnDisconnect(Conn : TPcbisConnection);
var
  Idx : integer;
begin
  Idx := GetSession(Conn);
  if Assigned(Sessions[Idx].FileStream) then
  begin
    Sessions[Idx].FileStream.Free;
    Sessions[Idx].FileStream := nil;
    LogWarn('BINKP: incomplete file transfer from ' + Conn.RemoteIP);
  end;
end;

end.
