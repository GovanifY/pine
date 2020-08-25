pipeline {
    agent { docker { image 'nixos/nix' } }
    stages {
        stage('build') {
            steps {
                sh '''
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


