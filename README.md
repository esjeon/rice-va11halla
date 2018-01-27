
Eon's VA-11 HALL-A Rice
=======================

This is my [VA11 HALL-A][1] themed rice for linux, using [suckless][2] tools.

![rice shot](/screenshot.png)
![main screen](/screenshot-empty.png)
![with terminals](/screenshot-terms.png)
![with vscode](/screenshot-code.png)
![with web browser](/screenshot-web.png)


Resources
---------
 * Background: a modified version of ["VA-11 HALL-A: Jill" by makaroll410][3]
 * Fonts (*NOT* included)
   - [Ubuntu](https://design.ubuntu.com/font/)
   - [NanumGothic](http://hangeul.naver.com/font) (for Korean)
   - [Font Awesome 5 Free](http://fontawesome.io/)


Software (Included)
-------------------

 * window manager: [dwm](https://dwm.suckless.org)
   - [patches](https://dwm.suckless.org/patches/) applied: *tilegap*, *status2d*, *alpha*
   - modified to allow adjusting bar paddings
   - *status2d* is customized to support graph drawing

 * app launcher: [dmenu](https://tools.suckless.org/dmenu/)
   - [patches](https://tools.suckless.org/dmenu/patches/) applied: *line-height*
   - a little hack is applied to keep process tree cleaner

 * status monitor: [slstatus](https://github.com/drkhsh/slstatus)


Software (Recommended)
-------------------

 * X compositor: [compton](https://github.com/chjj/compton)

 * terminal emulator: [st](https://st.suckless.org)
   - [alpha patch](https://st.suckless.org/patches/alpha/) makes it translucent.


Install
-------

Simple `make install` will install all included programs under `${HOME}/.local`,
and, therefore, executables will be placed in `${HOME}/.local/bin`.

You may edit `config.h` to customize the programs, and must re-run
`make install` afterwards.

Use/edit included `xinitrc` or write your own startup script. You're on your own
if you want to use other methods.

Uninstalling is as easy as simple `make uninstall`.


[1]: http://waifubartending.com/
[2]: https://suckless.org/
[3]: https://koyorin.deviantart.com/art/VA-11-HALL-A-Jill-621458694
