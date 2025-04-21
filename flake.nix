{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.11";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
      ...
    }@attrs:
    flake-utils.lib.eachSystem flake-utils.lib.defaultSystems (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            clang
            clang-tools
            openmpi
            (python3.withPackages (ps: with ps; [
              matplotlib
              numpy
            ]))
            graphviz
          ];
          buildInputs = with pkgs; [
            openmpi
            pkg-config
          ];
        };
      }
    );
}

