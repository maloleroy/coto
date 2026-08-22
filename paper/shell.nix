{ pkgs ? import <nixpkgs> { } }:

pkgs.mkShell {
  packages = [
    (pkgs.texliveSmall.withPackages (ps: with ps; [
      acmart
      xstring
      iftex
      etoolbox
      trimspaces
      environ
      hyperxmp
      ifmtarg
      totpages
      ncctools
      libertine
      newtx
      inconsolata
      microtype
      caption
      comment
      booktabs
      preprint
    ]))
    pkgs.gnumake
  ];
}
