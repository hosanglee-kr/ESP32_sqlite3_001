name: Firmware release

on:
  release:
    types: [published]

jobs:
  build:
    strategy:
      fail-fast: false
      matrix:
        environment:
          - "esp32doit"
          - "esp32S3_n4r2"

    runs-on: ubuntu-latest

    steps:
      - uses: actions/checkout@v4
      - name: Cache pip
        uses: actions/cache@v4
        with:
          path: ~/.cache/pip
          key: ${{ runner.os }}-pip-${{ hashFiles('**/requirements.txt') }}
          restore-keys: |
            ${{ runner.os }}-pip-

      - name: Cache PlatformIO
        uses: actions/cache@v4
        with:
          path: ~/.platformio
          key: ${{ runner.os }}-${{ hashFiles('**/lockfiles') }}

      - name: Set up Python
        uses: actions/setup-python@v5
        with:
            python-version: '3.11'

      - name: Install PlatformIO
        run: |
          python -m pip install --upgrade pip
          pip install --upgrade platformio

    
      - name: Extract build version
        id: get_version
        uses: battila7/get-version-action@v2.2.1
        with:
          # 이 작업에서 환경 변수로 버전을 반환할 수 있도록 수정 필요
          # (작업의 입력값이 필요할 수 있으니 확인)
          node-version: '20' # Node.js 버전 설정
          
      - name: Set version environment variable
        run: echo "VERSION=${{ steps.get_version.outputs.version-without-v }}" >> $GITHUB_ENV

      - name: Run PlatformIO for ${{ matrix.environment }}
        env:
          VERSION: ${{ env.VERSION }}
        run: pio run -e ${{ matrix.environment }}  # 각 환경에 맞는 빌드 실행


      - name: Archive production artifacts for ${{ matrix.environment }}
        uses: actions/upload-artifact@v4
        with:
          name: firmware-${{ matrix.environment }}
          path: .pio/build/${{ matrix.environment }}/firmware*.bin
          #path: .pio/build/**/*.bin
          

      - name: Release for ${{ matrix.environment }}
        uses: softprops/action-gh-release@v1
        env:
          GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
        with:
          files: .pio/build/${{ matrix.environment }}/firmware*.bin  # 각 환경의 firmware.bin 추가
          tag_name: ${{ github.ref_name }}  # release에 맞는 tag 설정
          name: Firmware Release ${{ matrix.environment }}  # 각 환경에 맞는 이름 설정
          body: "Firmware for ${{ matrix.environment }} environment"
          draft: false
          prerelease: false
          
        #with:
        #  files: .pio/build/${{ matrix.environment }}/firmware*.bin
        #  #files: .pio/build/**/firmware*.bin
