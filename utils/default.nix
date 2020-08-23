{ pkgs ? import <nixpkgs> {}
}:
pkgs.mkShell {
  name = "pcsx2ipc";
  buildInputs = [
    pkgs.doxygen
    pkgs.gnumake
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
    pkgs.catch2
    pkgs.gcovr
  ];

    shellHook = ''
      export CXX="clang++"
      export CARGO_HOME=$HOME/.cache/cargo
      export PCSX2_TEST="/tmp/pcsx2_debug/bin/PCSX2 --nogui ~/Documents/projects/programming/hacking/games/KINGDOM_HEARTS/KH2FM/KH2FM.ISO"
    '';
}
