with import <nixpkgs> {};

clangStdenv.mkDerivation {
  name = "thunderstorm-shell";

  buildInputs = with pkgs; [
     clang cmake lightning fmt gnumake lldb gbenchmark
  ];
}
