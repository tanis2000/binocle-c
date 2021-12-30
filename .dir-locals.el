((nil . ((cmake-ide-build-dir . "./cmake-build-debug"))))
(setq projectile-project-configure-cmd . "cd cmake-build-debug && cmake ../")
(setq projectile-project-compilation-cmd . "cd cmake-build-debug && make -j8")
(setq projectile-project-run-cmd "cd cmake-build-debug && ./example/ExampleProject.app/Contents/MacOS/ExampleProject")
