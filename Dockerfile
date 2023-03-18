FROM fedora:37 AS development

ARG UID=1001
ARG GID=1001

RUN sed -e 's|^metalink=|#metalink=|g' \
             -e 's|^#baseurl=http://download.example/pub/fedora/linux|baseurl=https://mirrors.tuna.tsinghua.edu.cn/fedora|g' \
             -i.bak \
             /etc/yum.repos.d/fedora.repo \
             /etc/yum.repos.d/fedora-modular.repo \
             /etc/yum.repos.d/fedora-updates.repo \
             /etc/yum.repos.d/fedora-updates-modular.repo \
    && dnf install -y \
        openssh \
        openssh-server \
        llvm    \
        llvm-devel \
        gcc     \
        g++     \
        gdb     \
        cmake   \
        rsync   \
        tar     \
        passwd  \
        zlib-devel

RUN groupadd -r --gid $GID llvm     \
    && useradd -m -u $UID -g $GID llvm \
    && yes passwd | passwd llvm     \
    && mkdir -p /etc/sudoers.d      \
    && echo "llvm ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/llvm \
    && chmod 0440 /etc/sudoers.d/llvm

RUN ( \
    echo 'LogLevel DEBUG2'; \
    echo 'PermitRootLogin yes'; \
    echo 'PasswordAuthentication yes'; \
    echo 'Subsystem sftp /usr/libexec/openssh/sftp-server'; \
  ) > /etc/ssh/sshd_config_test_clion \
  && mkdir /run/sshd \
  && ssh-keygen -A

CMD ["/usr/sbin/sshd", "-D", "-e", "-f", "/etc/ssh/sshd_config_test_clion"]