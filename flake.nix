{
  description = "INET Framework for OMNeT++ Discrete Event Simulator";

  inputs = {
    # omnetpp.url = "/home/rhornig/omnetpp-dev";
    omnetpp.url = "github:omnetpp/omnetpp/flake/omnetpp-6.0.x";
    flake-utils.follows = "omnetpp/flake-utils";
  };

  outputs = { self, flake-utils, omnetpp }:
    flake-utils.lib.eachDefaultSystem(system:
    let
      oppPkgs = omnetpp.oppPkgs.${system};
      pname = "inet";
      version = "4.4.1.${oppPkgs.lib.substring 0 8 self.lastModifiedDate}.${self.shortRev or "dirty"}";
    in rec {
      packages = rec {
        ${pname} = oppPkgs.callPackage ./releng/inet_mkDerivation.nix {
          inherit pname version;
          omnetpp = omnetpp.packages.${system}.omnetpp;
          src = self;
        };

        default = packages.${pname};
      };

      devShells = rec {
        "${pname}" = oppPkgs.stdenv.mkDerivation {
          name = "inet";
          buildInputs = [
            omnetpp.packages.${system}.default
            self.packages.${system}.default
          ];
          shellHook = ''
            source ${self.packages.${system}.default}/setenv
          '';
        };

        default = devShells."${pname}";
      };
    });
}