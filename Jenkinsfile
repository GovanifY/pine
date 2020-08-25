pipeline {
    agent { docker { image 'nixos/nix' } }
    stages {
        stage('build') {
            steps {
                sh 'nix --version'
            }
        }
    }
}


