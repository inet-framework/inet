{ 
  pname, version, src ? ./.,             # direct parameters
  stdenv, lib, pkg-config,               # build environment
  perl, omnetpp,                  # dependencies
  z3, ffmpeg,
}:
let
in
stdenv.mkDerivation rec {
  inherit pname version src;

  enableParallelBuilding = true;
  strictDeps = true;
  dontStrip = true;

  buildInputs = [ omnetpp z3 ffmpeg ];

  # tools required for build only (not needed in derivations)
  nativeBuildInputs = [ omnetpp pkg-config ];

  # we have to patch all shebangs to use NIX versions of the interpreters
  prePatch = ''
    patchShebangs bin
  '';

  buildPhase = ''
    source setenv
    opp_featuretool reset
    make makefiles
    make MODE=release -j16
  '';

  installPhase = ''
    runHook preInstall

    mkdir -p ${placeholder "out"}
    # get rid of precompiled headers to remove dependency from gcc/clang
    rm -f src/inet/common/precompiled_release.h.pch* src/inet/common/precompiled_debug.h.pch*

    installFiles=(bin src python Makefile setenv .oppbuildspec .oppfeatures .oppfeaturestate)
    for f in ''${installFiles[@]}; do
      cp -r $f ${placeholder "out"}
    done
    echo "src" >${placeholder "out"}/.nedfolders
    grep -E -v 'inet.examples|inet.showcases|inet.tutorials' .nedexclusions >${placeholder "out"}/.nedexclusions

    runHook postInstall
    '';

  shellHook = ''
    source $out/setenv
  '';

  meta = with lib; {
    homepage= "https://inet.omnetpp.org";
    description = "INET Framework for OMNeT++ Discrete Event Simulator";
    longDescription = "An open-source OMNeT++ model suite for wired, wireless and mobile networks. INET evolves via feedback and contributions from the user community.";
    changelog = "https://github.com/inet-framework/inet/blob/v${version}/WHATSNEW";
    # license = licenses.lgpl3;
    maintainers = [ "rudi@omnetpp.org" ];
    platforms = [ "x86_64-linux" "x86_64-darwin" ];
  };
}