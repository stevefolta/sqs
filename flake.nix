{
	description = "Quick scripting language";

	inputs = {
		};

	outputs = { self, nixpkgs }:
		let
			# Why doesn't the flakes build system handle this automatically?!
			forAllSystems = nixpkgs.lib.genAttrs nixpkgs.lib.systems.flakeExposed;
			nixpkgsFor = forAllSystems (system: import nixpkgs { inherit system; });
		in {
			packages = forAllSystems (system: {
				default =
					nixpkgsFor.${system}.stdenv.mkDerivation {
						name = "qqs";
						src = self;
						buildInputs = with nixpkgsFor.${system}; [ boehmgc ];
						installPhase = ''
							mkdir -p $out/bin
							cp qqs $out/bin/
							'';
						};
					});
			};
}


