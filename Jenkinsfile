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
                rm -rf /tmp/reports
                mkdir /tmp/reports
                if [ -d nix-cache ]
                then
                    rm -rf /nix && ln -s nix-cache /nix
                fi
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
                nix-shell --run "cd ../build && ./tests -r junit -o /tmp/reports/pcsx2.junit"
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
            sh '''
            if [ ! -d nix-cache ]
            then
                mv /nix nix-cache
            fi
            '''
            archiveArtifacts artifacts: 'release/*', fingerprint: true
            junit '/tmp/reports/*.junit'
        }
    }
}
