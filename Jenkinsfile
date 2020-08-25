pipeline {
    agent { 
        docker { 
        image 'nixos/nix' 
        args '-u root --privileged' 
        } 
    }
    stages {
        stage('build') {
            steps {
                sh '''
                nix-channel --add https://nixos.org/channels/nixpkgs-unstable nixpkgs
                nix-channel --update
                cd utils/
                nix-shell
                cd ../
                meson build
                cd build
                ninja
                '''
            }
        }
    }
}
