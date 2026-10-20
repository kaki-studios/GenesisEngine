{
  description = "GenesisEngine dev environment";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs =
    { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };

      # Libraries SDL3/bgfx dlopen at runtime; must be on the loader path on NixOS.
      runtimeLibs = with pkgs; [
        wayland
        libxkbcommon
        libdecor
        dbus
        systemd
        alsa-lib
        libpulseaudio
        libGL
        vulkan-loader
        libx11
        libxext
        libxcursor
        libxrandr
        libxi
        libxfixes
        libxrender
        libxcb
        libxscrnsaver
      ];

      buildTools = with pkgs; [
        cmake
        ninja
        pkg-config
        git
        kdePackages.extra-cmake-modules
      ];

      # Headers needed to compile bgfx/SDL backends.
      devInputs = with pkgs; [
        wayland-protocols
        wayland-scanner
        vulkan-headers
        libxcb-util
        libxcb-keysyms
        libxcb-wm
        libxcb-image
        libxcb-render-util
      ];
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        name = "GenesisEngine";

        packages = buildTools ++ runtimeLibs ++ devInputs;

        shellHook = ''
          export LD_LIBRARY_PATH=${pkgs.lib.makeLibraryPath runtimeLibs}''${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
          export CMAKE_EXPORT_COMPILE_COMMANDS=ON
        '';
      };
    };
}
