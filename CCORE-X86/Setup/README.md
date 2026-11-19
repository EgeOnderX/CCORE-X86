# CCORE Setup

    make run        # builds setup.img and boots it in QEMU

CCORE starts normally (POST, loading screen), then the Setup wizard opens:
resolution (320x200 VGA / 640x480 / 720x480), administrator password, user account,
date and time (written to the RTC), personalization, then it installs CCORE onto the disk.
After "Restart" the same image boots the installed system.
Run `make reinstall` to start over with a fresh disk.
Requires the CCORE-X86 folder next to this one.
