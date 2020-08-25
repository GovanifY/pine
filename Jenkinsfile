pipeline {
    agent { 
        docker { 
        image 'nixos/nix' 
        args '-u root --privileged -v /nix' 
        } 
    }
    stages {
        stage('build') {
            steps {
                sh '''
                nix-channel --add https://nixos.org/channels/nixpkgs-unstable nixpkgs
                nix-channel --update
                cd utils/
                nix-shell --run "cd ../ && meson build && cd build && ninja"
                '''
            }
        }
    }
}
