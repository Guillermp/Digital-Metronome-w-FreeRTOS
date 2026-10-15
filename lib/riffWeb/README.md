# Wi-Fi configuration

Before building a fresh checkout, copy `wifi_secrets.example.h` to
`wifi_secrets.h` in this directory and enter your Wi-Fi SSID and password.
The local `wifi_secrets.h` file is ignored by Git; commit only the example.

The credentials are still compiled into the device firmware. Git ignore rules
do not remove credentials from earlier commits. If a password was published,
change it on the Wi-Fi network and update your local `wifi_secrets.h`.
