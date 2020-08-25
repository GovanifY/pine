pipeline {
    agent { docker { image 'meson' } }
    stages {
        stage('build') {
            steps {
                sh 'meson --version'
                sh 'ninja --version'
            }
        }
    }
}


