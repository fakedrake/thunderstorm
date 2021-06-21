with import <nixpkgs> {};
let
  python = python38.withPackages(ps : with ps; [ matplotlib pygobject3 ipython ]);
in llvmPackages_12.stdenv.mkDerivation {
  name = "thunderstorm-shell";
  LOCALE_ARCHIVE_2_27 = "${glibcLocales}/lib/locale/locale-archive";
  buildInputs = with pkgs; [
     clang cmake lightning fmt gnumake lldb gbenchmark perf-tools python
  ];
}
