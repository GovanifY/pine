{ pkgs ? import <nixpkgs> {}
}:
let 
  pcsx2-ipc = pkgs.pcsx2.overrideAttrs (oldAttrs: rec {
        src = pkgs.fetchFromGitHub {
            owner = "GovanifY";
            repo = "pcsx2";
            rev = "0f659071f5ef7a0312b2c6b73061538249c72c28";
            sha256 = "19g0vmnzxi2wwp9iqpqc2n43qb5fnmlcimb7766afw60whyia2c1";
          };
        cmakeFlags =  oldAttrs.cmakeFlags ++ ["-DEXTRA_PLUGINS=TRUE"];
      });
in
pkgs.mkShell {
  name = "pcsx2ipc";
  buildInputs = [
    pcsx2-ipc
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
    pkgs.llvmPackages.bintools
    pkgs.catch2
    pkgs.pkgconfig
    pkgs.xorg.xorgserver
    pkgs.meson
    pkgs.ninja
    pkgs.gcovr
  ];

  # about PCSX2_TEST:
  # probably a good idea to configure PCSX2 beforehand with the plugins and
  # enable console to stdio. I use Xvfb to make PCSX2 run headlessly 
  # on linux, I run it in build-release.sh

  # first wget is ISO built from v , second is manually handcrafted pcsx2 config hell
  # https://github.com/ps2homebrew/Open-PS2-Loader/releases/tag/0.9.3
  shellHook = ''
      export CXX=clang++
      # we download some homebrew for later
      if [[ ! -f "/tmp/opl.iso" ]]; then
          wget https://cloud.govanify.com/index.php/s/NN4wfkLwD6HFSgS/download
          mv download /tmp/opl.iso
      fi
            
      # we download pcsx2 conf
      if [[ ! -f "/tmp/conf.zip" ]]; then
          wget https://cloud.govanify.com/index.php/s/r2etFXb7C2ifQri/download
          mv download /tmp/conf.zip
      fi
      rm -rf ~/.config/PCSX2
      rm -rf PCSX2
      mkdir ~/.config
      unzip /tmp/conf.zip
      mv PCSX2 ~/.config

      # we set it up
      find ~/.config/PCSX2 -exec sed -i -e "s'NIXSTR'${pcsx2-ipc}'g" {} \;
      find ~/.config/PCSX2 -exec sed -i -e "s'/root'$HOME'g" {} \;
      find ~/.config/PCSX2 -exec sed -i -e "s'ConsoleToStdio=disabled'ConsoleToStdio=enabled'g" {} \;

      # rust binding
      export CARGO_HOME=$HOME/.cache/cargo

      # pcsx2 headless things
      find ~/.config/PCSX2 -exec sed -i -e "s'libGSdx-AVX2\.so'libGSnull.so'g" {} \;
      killall Xvfb
      Xvfb :99 &
      export DISPLAY=:99
      export PCSX2_TEST="${pcsx2-ipc}/bin/PCSX2 /tmp/opl.iso"
    '';
}
