----------------------------------------------------------------
    FontPack Tool 모음 (2016.11.14)
        By 서영만 <email:sharksym@hitel.net>
----------------------------------------------------------------

* UPDATE

  - ANSI 스크린 v1.0
    ANSI1/2, 하단 펑션키 리스트 추가
    ANSI1/2, WIDTH 42 ~ 64 설정 가능
    커맨드 도움말 추가
    작은 버그들 수정


* BIOS UPDATE 방법

  1. STARTUP.ROM 및 BIOS.ROM 파일을 디스크로 복사
  2. FONTPACK B STARTUP.ROM 명령으로 다운로드
  3. 시스템 리셋
  4. FONTPACK B BIOS.ROM 명령으로 다운로드
  5. 시스템 리셋
  6. 끝



[ FONTPACK.COM ] -----------------------------------------------

  FontPack의 플래쉬롬/SRAM에 롬을 다운로드 또는
  ANSI 스크린, IME 설정을 위한 MSX-DOS 프로그램입니다.


* 요구 사항

  FontPack 카트리지
  MSX-DOS1/2


* 사용법

  FONTPACK W|WS|R|RS filename slot[subslot]

    W:  플래쉬롬 다운로드 (FontPack 롬 또는 ASCII 16K 메가롬)
    R:  플래쉬롬 덤프
    WS: SRAM 다운로드 (USER FONT DATA)
    RS: SRAM 덤프

  예) FONTPACK W  FONTPACK.ROM 1
      FONTPACK WS SRAM.ROM 2
      FONTPACK R  DUMP.ROM 23
      FONTPACK RS DUMP.ROM 2


  FONTPACK B|U|X filename

    B:  STARTUP 또는 BIOS 다운로드
    U:  USER롬 다운로드 (ASCII 16K 1메가롬)
    X:  SRAM 다운로드 및 실행 (ASCII 16K 1메가롬)

  예) FONTPACK B STARTUP.ROM
      FONTPACK B BIOS.ROM
      FONTPACK U ZANACEX.ROM
      FONTPACK X ZANACEX.ROM


  FONTPACK F [filename]

    F: FNT 파일(2KBytes 8x8 font)을 MAIN BIOS 용 Font로 설정
       FNT 파일 지정하지 않으면, MAIN BIOS의 기본 Font로 리셋

  예) FONTPACK F MYFONT.FNT


  FONTPACK ANSI[0|1|2|3]

    ANSI:  현재 설정 표시
    ANSI0: ANSI 스크린 해제
    ANSI1: 64x24 CP949 전환
    ANSI2: 64x24 JOHAB 전환
    ANSI3: 85x35 CP949 전환

  예) FONTPACK ANSI3


  FONTPACK IME[0|1]

    IME:  현재 설정 표시
    IME0: 한글 IME 해제
    IME1: 한글 IME 사용

  예) FONTPACK IME1



[ FontPackTool.exe ] -------------------------------------------

  FontPack 용 ROM파일을 생성하는 Windows 프로그램입니다.


* 요구 사항

  MS Windows 7 이상
  *.FNT 파일


* 사용법

  1. FontPackTool.exe 및 각종 폰트(*.FNT) 파일을 준비
  2. FontPackTool 실행 후, 16x16 및 12x12 폰트 설정
     TTF 폰트 사용 시 크기는 12pt, 9pt로 설정
  3. STARTUP/BIOS/USER ROM파일 설정
     USER ROM파일은 ASCII16K 128KB 롬파일 사용
  4. "롬파일 저장" 버튼으로 최종 ROM파일 생성
  5. FONTPACK.COM 이용하여 생성된 롬파일을 다운로드


* 기본 FNT 파일

  0000_00FF_ASCII.FNT -> 16x16 ASCII 폰트
  JOHAB_844.FNT       -> 16x16 한글 844 조합폰트
  A1A1_AEFE_SPE.FNT   -> 16x16 특수문자 폰트
  CAA1_FDFE_HANJA.FNT -> 16x16 한자 폰트



[ STARTUP / BIOS ] ---------------------------------------------

  FontPack롬에 포함되는 SW(롬)입니다.
  FontPackTool.exe 프로그램에서 FontPack 롬에 삽입합니다.


* 파일

  STARTUP.ROM  -> 부트코드 v1.04
  BIOS.ROM     -> 한글 IME v0.8, ANSI 스크린 v0.9



[ BASIC Command ] ----------------------------------------------

* 아스키 문자 크기 및 한글 코드 설정

  SET HAN 0 -> 아스키 폰트 8 x  8, 조합형 한글 16 x 16
  SET HAN 1 -> 아스키 폰트 8 x 16, 조합형 한글 16 x 16
  SET HAN 2 -> 아스키 폰트 8 x  8, 완성형 한글 16 x 16
  SET HAN 3 -> 아스키 폰트 8 x 16, 완성형 한글 16 x 16

* 그래픽 모드(SCREEN 2~12) 용 문자 출력 디바이스

  OPEN "FNP:" AS #1
  PRINT #1, "한글 TEST"
