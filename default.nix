{ pkgs ? import <nixpkgs> {}
}:
pkgs.mkShell {
  name = "pcsx2ipc";
  buildInputs = [
    pkgs.doxygen
    pkgs.gnumake
    pkgs.gcc
    pkgs.python3
    (pkgs.texlive.combine { inherit (pkgs.texlive) scheme-medium varwidth
    multirow hanging adjustbox collectbox stackengine sectsty tocloft
    newunicodechar etoc; })
  ];
}
