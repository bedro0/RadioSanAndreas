# Radio San Andreas


This repo contains a set of scripts that allow you to play radio stations from Grand Theft Auto: San Andreas over web, in real life.

Prerequisite:

1. Download [San Andreas Audio Toolkit](https://github.com/bedro0/Project-gRadio/raw/main/San%20Andreas%20Audio%20Toolkit.7z). 
2. Extract the archive and open SaatGuiFrontend.exe inside the folder named Alci_s_SAAT_GUI_FrontEnd_1.0.
3. Click File ==> Set Working Directory, to choose the folder where all the Audio files will be exported.
4. Click File ==> Set GTA Directory, and choose the folder with original GTA San Andreas installed.
5. To export radio tracks, simply double click the labels under the tab labeled "Archiv". The files are simply going to appear in the working directory. This project now supports all channels from the game.

Steps to install:
1. Clone/Download the repo
2. `docker build -t radiosa .` inside the downloaded dir
3. `docker run -d --volume [path-to-music-files-exported-from-game]:/radiosa/music -p 8000:8000 -p 3000:3000 radiosa` (you can omit exposing port 8000)
4. Go to localhost:3000 for new custom front end interface (recommended) or localhost:8000 for Icecast interface.
5. Enjoy!

**Environmental Variables (Only For Advanced Users):**

| Variable            | Purpose                                                          | Default Value                                                                       |
| ------------------- | ---------------------------------------------------------------- | ----------------------------------------------------------------------------------- |
| HOSTNAME            | Hostname for Icecast. (As far as I know, it's only for display.) | localhost                                                                           |
| DISPLAY_ADMIN       | Display name for an administrator on Icecast version page        | admin                                                                               |
| ADMIN_USER          | Username for administrator login                                 | admin                                                                               |
| ENABLED_STATIONS    | Enabled Radio Stations                                           | "bouncefm, csr, kdst, kjah, krose, mastersounds, playbackfm, radiols, radiox, sfur" |
| ICECAST_SOURCE_PASS | Source Password for Icecast (ShoutCast) stream                   | (Random string of 16 CAPITAL alphanumeric characters)                                              |
| ICECAST_RELAY_PASS  | Relay Password for Icecast                                       | (Random string of 16 CAPITAL alphanumeric characters)                                              |
| ICECAST_ADMIN_PASS  | Administrator password for Icecast web login                     | (Random string of 16 CAPITAL alphanumeric characters)                                              |


Additional Resources

[Icecast Documentation](https://icecast.org/docs/icecast-2.4.1/)

[MPD Documentation (Arch Wiki)](https://wiki.archlinux.org/title/Music_Player_Daemon)

[MPD Configuration (Fandom) - Adblock Recommended while browsing Fandom Website](https://mpd.fandom.com/wiki/Configuration)

[MPD Website](https://www.musicpd.org/)

