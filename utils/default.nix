{ pkgs ? import <nixpkgs> {}
}:
pkgs.mkShell {
  name = "pcsx2ipc";
  buildInputs = [
    pkgs.doxygen
    pkgs.gnumake
    pkgs.gcc
    pkgs.clang
    pkgs.python3
    pkgs.zip
    pkgs.clang-tools
    (pkgs.texlive.combine { inherit (pkgs.texlive) scheme-medium varwidth
    multirow hanging adjustbox collectbox stackengine sectsty tocloft
    newunicodechar etoc; })
    pkgs.dotnet-sdk_3
    pkgs.pythonPackages.pip
    pkgs.cargo
    pkgs.rustc
    pkgs.luajit
    pkgs.gcovr
    pkgs.catch2
    pkgs.pkgconfig
    pkgs.xorg.xorgserver
    pkgs.meson
  ];

  # about PCSX2_TEST:
  # probably a good idea to configure PCSX2 beforehand with the plugins and
  # enable console to stdio. I use Xvfb to make PCSX2 run headlessly 
  # on linux, I run it in build-release.sh
  shellHook = ''
      export DISPLAY=:99
      export CARGO_HOME=$HOME/.cache/cargo
      export PCSX2_TEST="/tmp/pcsx2_debug/bin/PCSX2 ~/Documents/projects/programming/hacking/games/KINGDOM_HEARTS/KH2FM/KH2FM.ISO"
    '';
}
