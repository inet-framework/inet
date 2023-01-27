{
  description = "INET Framework for OMNeT++ Discrete Event Simulator";

  inputs = {
    # omnetpp.url = "/home/rhornig/omnetpp-dev";
    omnetpp.url = "github:omnetpp/omnetpp/v/6.0.x";
    flake-utils.follows = "omnetpp/flake-utils";
  };

  outputs = { self, flake-utils, omnetpp }:
    flake-utils.lib.eachDefaultSystem(system:
    let
      oppPkgs = omnetpp.oppPkgs.${system};
      pname = "inet";
      githash = self.shortRev or "dirty";
      timestamp = oppPkgs.lib.substring 0 8 self.lastModifiedDate;
      gversion = "${githash}.${timestamp}";
      sversion = "6.0.1"; # schemantic version number

    in rec {
      packages = rec {
        default = packages.${pname};

        ${pname} = oppPkgs.callPackage ./releng/inet_mkDerivation.nix {
          inherit pname; 
          version = gversion;
          omnetpp = omnetpp.packages.${system}.omnetpp-runtime;
          src = self;
        };
      };

      devShells = rec {
        default = oppPkgs.stdenv.mkDerivation {
          name = "${pname}-${gversion} dependencies";
          buildInputs = self.packages.${system}."${pname}".buildInputs;
          nativeBuildInputs = self.packages.${system}."${pname}".nativeBuildInputs;

          shellHook = ''
            source setenv
          '';
        };
      };

    });
}