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
  ];
    # clang is pretty nice
    shellHook = ''
      export CXX="clang++"
    '';
}
