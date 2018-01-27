#!/bin/bash

export PATH="${HOME}/bin:${HOME}/.local/bin:${PATH}"

cd "${HOME}"

feh --bg-fill ${HOME}/.local/share/backgrounds/va11halla-background.png
compton &
slstatus &
exec dwm
