pipeline {
    agent { 
        docker { 
            image 'nixos/nix' 
            args '-u root --privileged -v $HOME/nix-cache:/nix' 
        } 
    }
    stages {
        stage('build') {
            steps {
                sh '''
                rm -rf /tmp/reports
                mkdir /tmp/reports
                nix-channel --add https://nixos.org/channels/nixpkgs-unstable nixpkgs
                nix-channel --update
                cd utils/
                nix-shell --run "cd ../ && meson build && cd build && ninja"
                '''
            }
        }
        // we could probably test the IPC with different emulators here in
        // parallel when that's a thing
        stage('tests') {
            steps {
                sh '''
                cd utils/
                nix-shell --run "cd ../build && ./tests -r junit -o /tmp/reports/pcsx2.xml"
                '''
            }
        }
        stage('release') {
            steps {
                sh '''
                cd utils/
                nix-shell --run "sh -c ./build-release.sh"
                '''
            }
        }
    }
    post {
        always {
            archiveArtifacts artifacts: 'release/*', fingerprint: true
            junit '/tmp/reports/pcsx2.xml'
        }
    }
}
